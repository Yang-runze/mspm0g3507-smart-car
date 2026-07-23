#include "motor_speed_pid_test.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "delay.h"
#include "encoder_user.h"
#include "hal_uart.h"
#include "motor_user.h"
#include "oled_driver.h"
#include "board_led.h"
#include "systick.h"

typedef struct {
    float integral_output;
    float last_error;
    float filtered_rpm;
    bool derivative_ready;
} motor_speed_pid_controller_t;

static float motor_speed_pid_clamp(float value, float minimum, float maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static float motor_speed_pid_absolute(float value)
{
    return value < 0.0f ? -value : value;
}

static int motor_speed_pid_calculate(motor_speed_pid_controller_t *pid,
    float target_rpm, float measured_rpm, float dt_seconds,
    float kp, float ki, float kd, float *error_out)
{
    float error = target_rpm - measured_rpm;
    float derivative = 0.0f;
    float feedforward_output =
        MOTOR_SPEED_PID_FEEDFORWARD_PWM_PER_RPM * target_rpm;
    float integral_candidate;
    float unsaturated_output;
    float output;

    if (pid->derivative_ready && dt_seconds > 0.0f) {
        derivative = (error - pid->last_error) / dt_seconds;
        derivative = motor_speed_pid_clamp(derivative,
            -MOTOR_SPEED_PID_DERIVATIVE_LIMIT,
            MOTOR_SPEED_PID_DERIVATIVE_LIMIT);
    }

    integral_candidate = motor_speed_pid_clamp(
        pid->integral_output + (ki * error * dt_seconds),
        -MOTOR_SPEED_PID_INTEGRAL_OUTPUT_LIMIT,
        MOTOR_SPEED_PID_INTEGRAL_OUTPUT_LIMIT);
    unsaturated_output =
        feedforward_output + (kp * error) +
        integral_candidate + (kd * derivative);
    output = motor_speed_pid_clamp(unsaturated_output, 0.0f,
        (float) MOTOR_SPEED_PID_MAX_PWM);

    /*
     * Conditional integration: accept the new integral when the total output
     * is not saturated, or when the current error helps leave saturation.
     */
    if ((unsaturated_output == output) ||
        ((unsaturated_output > output) && (error < 0.0f)) ||
        ((unsaturated_output < output) && (error > 0.0f))) {
        pid->integral_output = integral_candidate;
    } else {
        output = motor_speed_pid_clamp(
            feedforward_output + (kp * error) +
            pid->integral_output + (kd * derivative),
            0.0f, (float) MOTOR_SPEED_PID_MAX_PWM);
    }

    if (output > 0.0f && output < (float) MOTOR_SPEED_PID_DEAD_ZONE_PWM) {
        output = (float) MOTOR_SPEED_PID_DEAD_ZONE_PWM;
    }

    pid->last_error = error;
    pid->derivative_ready = true;
    *error_out = error;
    return (int) output;
}

static float motor_speed_pid_counts_to_rpm(int32_t delta_count,
    uint32_t dt_ms, float encoder_sign)
{
    if (dt_ms == 0U) {
        return 0.0f;
    }

    return ((float) delta_count * encoder_sign * 60000.0f) /
        (MOTOR_SPEED_PID_ENCODER_COUNTS_PER_REV * (float) dt_ms);
}

static float motor_speed_pid_filter(motor_speed_pid_controller_t *pid,
    float raw_rpm)
{
    pid->filtered_rpm =
        (MOTOR_SPEED_PID_FILTER_KEEP * pid->filtered_rpm) +
        ((1.0f - MOTOR_SPEED_PID_FILTER_KEEP) * raw_rpm);
    return pid->filtered_rpm;
}

static int motor_speed_pid_tenths(float value)
{
    return (int) (value * 10.0f);
}

static float motor_speed_pid_get_target(uint32_t elapsed_ms)
{
#if MOTOR_SPEED_PID_ENABLE_TARGET_PROFILE
    uint32_t step =
        (elapsed_ms / MOTOR_SPEED_PID_STEP_HOLD_MS) % 3U;

    if (step == 1U) {
        return MOTOR_SPEED_PID_STEP_2_RPM;
    }
    if (step == 2U) {
        return MOTOR_SPEED_PID_STEP_3_RPM;
    }
#else
    (void) elapsed_ms;
#endif

    return MOTOR_SPEED_PID_TARGET_RPM;
}

static void motor_speed_pid_display(const char *line1, const char *line2,
    const char *line3)
{
    u8g2_ClearBuffer(&u8g2);
    /* Normal PID tuning page: restore the original readable 6x10 font. */
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    u8g2_DrawStr(&u8g2, 0U, 12U, "MOTOR SPEED PID");
    u8g2_DrawStr(&u8g2, 0U, 28U, line1);
    u8g2_DrawStr(&u8g2, 0U, 42U, line2);
    u8g2_DrawStr(&u8g2, 0U, 56U, line3);
    u8g2_SendBuffer(&u8g2);
}

static void motor_speed_pid_format_rpm(char *text, size_t text_size,
    const char *label, float value)
{
    int tenths = motor_speed_pid_tenths(value);
    int magnitude = tenths < 0 ? -tenths : tenths;

    (void) snprintf(text, text_size, "%s%s%d.%d RPM", label,
        tenths < 0 ? "-" : "", magnitude / 10, magnitude % 10);
}

static void motor_speed_pid_start_countdown(void)
{
    char countdown_text[22];

    for (uint32_t seconds = MOTOR_SPEED_PID_START_DELAY_SECONDS;
         seconds > 0U; seconds--) {
        (void) snprintf(countdown_text, sizeof(countdown_text),
            "START IN %lu s", (unsigned long) seconds);
        motor_speed_pid_display("LIFT CAR FIRST", countdown_text,
            "RESET = STOP");
        delay_ms(1000U);
    }
}

static void motor_speed_pid_fault(const char *message, int status_code)
{
    int motor_pwms[2] = {0, 0};

    motor_set_pwms(motor_pwms);
    board_led_set(true);
    motor_speed_pid_display(message, "MOTOR STOPPED", "RESET AFTER CHECK");
    /* Keep all 13 VOFA+ FireWater channels numeric even on a fault. */
    usart_printf(DEBUG_UART_INST,
        "0,0,0,0,0,0,0,0,0,0,0,0,%d\n", status_code);

    for (;;) {
        motor_set_pwms(motor_pwms);
        board_led_toggle();
        delay_ms(100U);
    }
}

void motor_speed_pid_test_run(void)
{
    motor_speed_pid_controller_t left_pid = {0};
    motor_speed_pid_controller_t right_pid = {0};
    int motor_pwms[2] = {0, 0};
    int32_t previous_left_count;
    int32_t previous_right_count;
    uint32_t last_control_ms;
    uint32_t target_profile_start_ms;
    uint32_t telemetry_counter = 0U;
    uint32_t display_counter = 0U;
    uint32_t wrong_sign_cycles = 0U;
    uint32_t stall_cycles = 0U;
    char left_text[22];
    char right_text[22];
    char pwm_text[22];

    u8g2_Init();
    motor_init();
    motor_set_pwms(motor_pwms);
    encoder_application_init();
    systick_init();
    board_led_set(false);

    encoder_manager_write(&robot_encoder_manager,
        MOTOR_SPEED_PID_LEFT_ENCODER_INDEX, 0);
    encoder_manager_write(&robot_encoder_manager,
        MOTOR_SPEED_PID_RIGHT_ENCODER_INDEX, 0);
    motor_speed_pid_start_countdown();

    previous_left_count = encoder_manager_read(&robot_encoder_manager,
        MOTOR_SPEED_PID_LEFT_ENCODER_INDEX);
    previous_right_count = encoder_manager_read(&robot_encoder_manager,
        MOTOR_SPEED_PID_RIGHT_ENCODER_INDEX);
    last_control_ms = get_ms();
    target_profile_start_ms = last_control_ms;

    for (;;) {
        uint32_t now_ms = get_ms();
        uint32_t dt_ms = now_ms - last_control_ms;
        float target_rpm = motor_speed_pid_get_target(
            now_ms - target_profile_start_ms);
        int32_t current_left_count;
        int32_t current_right_count;
        int32_t left_delta;
        int32_t right_delta;
        float left_raw_rpm;
        float right_raw_rpm;
        float left_rpm;
        float right_rpm;
        float left_error;
        float right_error;

        if (dt_ms < MOTOR_SPEED_PID_CONTROL_PERIOD_MS) {
            delay_ms(1U);
            continue;
        }
        last_control_ms = now_ms;

        current_left_count = encoder_manager_read(&robot_encoder_manager,
            MOTOR_SPEED_PID_LEFT_ENCODER_INDEX);
        current_right_count = encoder_manager_read(&robot_encoder_manager,
            MOTOR_SPEED_PID_RIGHT_ENCODER_INDEX);
        left_delta = current_left_count - previous_left_count;
        right_delta = current_right_count - previous_right_count;
        previous_left_count = current_left_count;
        previous_right_count = current_right_count;

        left_raw_rpm = motor_speed_pid_counts_to_rpm(left_delta, dt_ms,
            MOTOR_SPEED_PID_LEFT_ENCODER_SIGN);
        right_raw_rpm = motor_speed_pid_counts_to_rpm(right_delta, dt_ms,
            MOTOR_SPEED_PID_RIGHT_ENCODER_SIGN);
        left_rpm = motor_speed_pid_filter(&left_pid, left_raw_rpm);
        right_rpm = motor_speed_pid_filter(&right_pid, right_raw_rpm);

        motor_pwms[0] = motor_speed_pid_calculate(&left_pid,
            target_rpm, left_rpm, (float) dt_ms / 1000.0f,
            MOTOR_SPEED_PID_LEFT_KP, MOTOR_SPEED_PID_LEFT_KI,
            MOTOR_SPEED_PID_LEFT_KD, &left_error);
        motor_pwms[1] = motor_speed_pid_calculate(&right_pid,
            target_rpm, right_rpm, (float) dt_ms / 1000.0f,
            MOTOR_SPEED_PID_RIGHT_KP, MOTOR_SPEED_PID_RIGHT_KI,
            MOTOR_SPEED_PID_RIGHT_KD, &right_error);
        motor_set_pwms(motor_pwms);

        if (left_rpm < -MOTOR_SPEED_PID_WRONG_SIGN_RPM ||
            right_rpm < -MOTOR_SPEED_PID_WRONG_SIGN_RPM) {
            wrong_sign_cycles++;
        } else {
            wrong_sign_cycles = 0U;
        }
        if (wrong_sign_cycles >= MOTOR_SPEED_PID_WRONG_SIGN_CYCLES) {
            motor_speed_pid_fault("ENCODER SIGN ERROR", -1);
        }

        if ((motor_pwms[0] >= MOTOR_SPEED_PID_STALL_PWM &&
             motor_speed_pid_absolute(left_rpm) < MOTOR_SPEED_PID_STALL_RPM) ||
            (motor_pwms[1] >= MOTOR_SPEED_PID_STALL_PWM &&
             motor_speed_pid_absolute(right_rpm) < MOTOR_SPEED_PID_STALL_RPM)) {
            stall_cycles++;
        } else {
            stall_cycles = 0U;
        }
        if (stall_cycles >= MOTOR_SPEED_PID_STALL_CYCLES) {
            motor_speed_pid_fault("ENCODER NO PULSE", -2);
        }

        board_led_set(true);
        display_counter++;
        if (display_counter >= MOTOR_SPEED_PID_DISPLAY_PERIOD_CYCLES) {
            display_counter = 0U;
            motor_speed_pid_format_rpm(left_text, sizeof(left_text),
                "L:", left_rpm);
            motor_speed_pid_format_rpm(right_text, sizeof(right_text),
                "R:", right_rpm);
            (void) snprintf(pwm_text, sizeof(pwm_text), "T:%d P:%d,%d",
                (int) target_rpm, motor_pwms[0], motor_pwms[1]);
            motor_speed_pid_display(left_text, right_text, pwm_text);
        }

        telemetry_counter++;
        if (telemetry_counter >= MOTOR_SPEED_PID_TELEMETRY_PERIOD_CYCLES) {
            telemetry_counter = 0U;
            /* VOFA+ FireWater: 13 numeric channels, no text/header. */
            usart_printf(DEBUG_UART_INST,
                "%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%lu,0\n",
                motor_speed_pid_tenths(target_rpm),
                motor_speed_pid_tenths(left_rpm),
                motor_speed_pid_tenths(right_rpm), motor_pwms[0],
                motor_pwms[1], motor_speed_pid_tenths(left_error),
                motor_speed_pid_tenths(right_error),
                (long) current_left_count, (long) current_right_count,
                (long) left_delta, (long) right_delta,
                (unsigned long) dt_ms);
        }
    }
}
