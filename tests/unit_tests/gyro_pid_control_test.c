#include "gyro_pid_control_test.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "delay.h"
#include "hal_uart.h"
#include "lengke_gyro.h"
#include "motor_user.h"
#include "oled_driver.h"
#include "board_led.h"

typedef struct {
    float target_yaw_deg;
    float angle_integral_deg_s;
    float rate_integral_deg;
} gyro_pid_controller_t;

static volatile float g_gyro_pid_requested_target_yaw_deg =
    GYRO_PID_INITIAL_TARGET_YAW_DEG;

static float gyro_pid_clamp_float(float value, float minimum, float maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static int gyro_pid_clamp_pwm(int value)
{
#if GYRO_PID_ALLOW_MOTOR_REVERSE
    if (value < -GYRO_PID_MAX_PWM) {
        return -GYRO_PID_MAX_PWM;
    }
#else
    if (value < 0) {
        return 0;
    }
#endif
    if (value > GYRO_PID_MAX_PWM) {
        return GYRO_PID_MAX_PWM;
    }
    return value;
}

static float gyro_pid_wrap_angle(float angle_deg)
{
    while (angle_deg > 180.0f) {
        angle_deg -= 360.0f;
    }
    while (angle_deg < -180.0f) {
        angle_deg += 360.0f;
    }
    return angle_deg;
}

static float gyro_pid_abs_float(float value)
{
    return value < 0.0f ? -value : value;
}

void gyro_pid_set_target_yaw(float target_yaw_deg)
{
    g_gyro_pid_requested_target_yaw_deg =
        gyro_pid_wrap_angle(target_yaw_deg);
}

float gyro_pid_get_target_yaw(void)
{
    return gyro_pid_wrap_angle(g_gyro_pid_requested_target_yaw_deg);
}

static void gyro_pid_reset(gyro_pid_controller_t *pid, float target_yaw_deg)
{
    pid->target_yaw_deg = gyro_pid_wrap_angle(target_yaw_deg);
    pid->angle_integral_deg_s = 0.0f;
    pid->rate_integral_deg = 0.0f;
}

static bool gyro_pid_can_integrate(float unsaturated_output,
    float saturated_output, float error)
{
    if (unsaturated_output == saturated_output) {
        return true;
    }

    if ((unsaturated_output > saturated_output) && (error < 0.0f)) {
        return true;
    }

    if ((unsaturated_output < saturated_output) && (error > 0.0f)) {
        return true;
    }

    return false;
}

static int gyro_pid_calculate(gyro_pid_controller_t *pid, float target_yaw_deg,
    float yaw_deg,
    float z_rate_dps, float *angle_error_out, float *target_rate_out)
{
    const float dt_s = (float) GYRO_PID_CONTROL_PERIOD_MS / 1000.0f;
    float target_change = gyro_pid_wrap_angle(
        target_yaw_deg - pid->target_yaw_deg);
    float angle_error;
    float angle_integral_candidate;
    float target_rate_unsaturated;
    float target_rate;
    float rate_error;
    float rate_integral_candidate;
    float turn_output_unsaturated;
    float turn_output;

    /*
     * A commanded heading step must not inherit integral accumulated for the
     * previous target. Tiny target adjustments keep the learned compensation.
     */
    if (gyro_pid_abs_float(target_change) >= 0.05f) {
        pid->angle_integral_deg_s = 0.0f;
        pid->rate_integral_deg = 0.0f;
    }
    pid->target_yaw_deg = gyro_pid_wrap_angle(target_yaw_deg);

    angle_error = gyro_pid_wrap_angle(pid->target_yaw_deg - yaw_deg);
    angle_integral_candidate = gyro_pid_clamp_float(
        pid->angle_integral_deg_s + (angle_error * dt_s),
        -GYRO_PID_ANGLE_INTEGRAL_LIMIT_DEG_S,
        GYRO_PID_ANGLE_INTEGRAL_LIMIT_DEG_S);
    target_rate_unsaturated =
        (GYRO_PID_ANGLE_KP * angle_error) +
        (GYRO_PID_ANGLE_KI * angle_integral_candidate);
    target_rate = gyro_pid_clamp_float(target_rate_unsaturated,
        -GYRO_PID_TARGET_RATE_LIMIT_DPS, GYRO_PID_TARGET_RATE_LIMIT_DPS);

    if (gyro_pid_can_integrate(
            target_rate_unsaturated, target_rate, angle_error)) {
        pid->angle_integral_deg_s = angle_integral_candidate;
    }

    rate_error = target_rate - z_rate_dps;
    rate_integral_candidate = gyro_pid_clamp_float(
        pid->rate_integral_deg + (rate_error * dt_s),
        -GYRO_PID_RATE_INTEGRAL_LIMIT_DEG,
        GYRO_PID_RATE_INTEGRAL_LIMIT_DEG);
    turn_output_unsaturated =
        (GYRO_PID_RATE_KP * rate_error) +
        (GYRO_PID_RATE_KI * rate_integral_candidate);
    turn_output = gyro_pid_clamp_float(turn_output_unsaturated,
        -(float) GYRO_PID_TURN_PWM_LIMIT,
        (float) GYRO_PID_TURN_PWM_LIMIT);

    if (gyro_pid_can_integrate(
            turn_output_unsaturated, turn_output, rate_error)) {
        pid->rate_integral_deg = rate_integral_candidate;
    }

    turn_output *= GYRO_PID_TURN_DIRECTION;

    *angle_error_out = angle_error;
    *target_rate_out = target_rate;

    return (int) turn_output;
}

static void gyro_pid_display(const char *line1, const char *line2,
    const char *line3)
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    u8g2_DrawStr(&u8g2, 0U, 12U, "GYRO CASCADE PID");
    u8g2_DrawStr(&u8g2, 0U, 28U, line1);
    u8g2_DrawStr(&u8g2, 0U, 42U, line2);
    u8g2_DrawStr(&u8g2, 0U, 56U, line3);
    u8g2_SendBuffer(&u8g2);
}

static void gyro_pid_format_pair(char *text, size_t text_size,
    const char *first_name, float first_value,
    const char *second_name, float second_value)
{
    int first_tenths = (int) (first_value * 10.0f);
    int second_tenths = (int) (second_value * 10.0f);
    int first_abs = first_tenths < 0 ? -first_tenths : first_tenths;
    int second_abs = second_tenths < 0 ? -second_tenths : second_tenths;

    (void) snprintf(text, text_size, "%s%s%d.%d %s%s%d.%d",
        first_name, first_tenths < 0 ? "-" : "", first_abs / 10,
        first_abs % 10, second_name, second_tenths < 0 ? "-" : "",
        second_abs / 10, second_abs % 10);
}

static bool gyro_pid_find_baud_rate(void)
{
    static const uint32_t baud_rates[] = {
        115200U, 9600U, 4800U, 19200U,
        38400U, 57600U, 230400U, 2400U
    };
    char baud_text[22];

    gyro_pid_display("WAIT MODULE 600ms", "UART1 RX = PA9", "MOTOR STOPPED");
    delay_ms(600U);

    for (size_t index = 0U;
         index < sizeof(baud_rates) / sizeof(baud_rates[0]); index++) {
        lengke_gyro_set_host_baud_rate(baud_rates[index]);
        (void) snprintf(baud_text, sizeof(baud_text), "TRY BAUD %lu",
            (unsigned long) baud_rates[index]);
        gyro_pid_display(baud_text, "SCANNING FRAMES", "MOTOR STOPPED");
        delay_ms(800U);

        if (lengke_gyro_has_valid_data()) {
            return true;
        }
    }

    return false;
}

static void gyro_pid_start_countdown(void)
{
    char countdown_text[22];
    char base_pwm_text[22];

    (void) snprintf(base_pwm_text, sizeof(base_pwm_text), "BASE PWM:%d",
        GYRO_PID_BASE_PWM);

    for (uint32_t seconds = GYRO_PID_START_DELAY_SECONDS;
         seconds > 0U; seconds--) {
        (void) snprintf(countdown_text, sizeof(countdown_text),
            "START IN %lu s", (unsigned long) seconds);
        gyro_pid_display("YAW ZERO READY", countdown_text, base_pwm_text);
        delay_ms(1000U);
    }
}

static int gyro_pid_tenths(float value)
{
    return (int) (value * 10.0f);
}

void gyro_pid_control_test_run(void)
{
    gyro_pid_controller_t pid = {0};
    int motor_pwms[2] = {0, 0};
    uint32_t last_frame_count;
    uint32_t stale_cycles = 0U;
    uint32_t display_counter = 0U;
    uint32_t telemetry_counter = 0U;
    char yaw_text[22];
    char rate_text[22];
    char motor_text[22];

    u8g2_Init();
    motor_init();
    motor_set_pwms(motor_pwms);
    lengke_gyro_init();
    board_led_set(false);

    if (!gyro_pid_find_baud_rate()) {
        board_led_set(false);
        gyro_pid_display("NO VALID FRAME", "CHECK TX -> PA9", "MOTOR STOPPED");
        for (;;) {
            motor_set_pwms(motor_pwms);
            delay_ms(100U);
        }
    }

    gyro_pid_display("GYRO FOUND", "ZEROING YAW...", "KEEP CAR STILL");
    sendCaliYawCommand();
    delay_ms(500U);
    gyro_pid_start_countdown();

    last_frame_count = lengke_gyro_get_valid_frame_count();
    gyro_pid_reset(&pid, gyro_pid_get_target_yaw());

    for (;;) {
        uint32_t frame_count = lengke_gyro_get_valid_frame_count();
        float target_yaw_deg = gyro_pid_get_target_yaw();
        float yaw_deg = Yaw();
        float z_rate_dps = GyroZ();
        float angle_error = 0.0f;
        float target_rate = 0.0f;
        int turn_pwm = 0;

        if (frame_count != last_frame_count) {
            stale_cycles = 0U;
            last_frame_count = frame_count;
        } else if (stale_cycles < GYRO_PID_DATA_TIMEOUT_CYCLES) {
            stale_cycles++;
        }

        if (stale_cycles < GYRO_PID_DATA_TIMEOUT_CYCLES) {
            turn_pwm = gyro_pid_calculate(&pid, target_yaw_deg,
                yaw_deg, z_rate_dps,
                &angle_error, &target_rate);
            motor_pwms[0] = gyro_pid_clamp_pwm(
                GYRO_PID_BASE_PWM - turn_pwm);
            motor_pwms[1] = gyro_pid_clamp_pwm(
                GYRO_PID_BASE_PWM + turn_pwm);
            board_led_set(true);
        } else {
            motor_pwms[0] = 0;
            motor_pwms[1] = 0;
            gyro_pid_reset(&pid, target_yaw_deg);
            board_led_set(false);
        }

        motor_set_pwms(motor_pwms);

        telemetry_counter++;
        if (telemetry_counter >= GYRO_PID_TELEMETRY_PERIOD_CYCLES) {
            telemetry_counter = 0U;
            /* VOFA+ FireWater: nine numeric channels, no text/header. */
            usart_printf(DEBUG_UART_INST,
                "%d,%d,%d,%d,%d,%d,%d,%d,%lu\n",
                gyro_pid_tenths(yaw_deg),
                gyro_pid_tenths(target_yaw_deg),
                gyro_pid_tenths(z_rate_dps),
                gyro_pid_tenths(target_rate),
                gyro_pid_tenths(angle_error), turn_pwm,
                motor_pwms[0], motor_pwms[1],
                (unsigned long) stale_cycles);
        }

        display_counter++;
        if (display_counter >= GYRO_PID_DISPLAY_PERIOD_CYCLES) {
            display_counter = 0U;
            gyro_pid_format_pair(yaw_text, sizeof(yaw_text),
                "Y:", yaw_deg, "T:", target_yaw_deg);
            gyro_pid_format_pair(rate_text, sizeof(rate_text),
                "W:", z_rate_dps, "R:", target_rate);

            if (stale_cycles < GYRO_PID_DATA_TIMEOUT_CYCLES) {
                (void) snprintf(motor_text, sizeof(motor_text), "L:%d R:%d",
                    motor_pwms[0], motor_pwms[1]);
            } else {
                (void) snprintf(motor_text, sizeof(motor_text),
                    "GYRO LOST - STOP");
            }
            gyro_pid_display(yaw_text, rate_text, motor_text);
        }

        delay_ms(GYRO_PID_CONTROL_PERIOD_MS);
    }
}
