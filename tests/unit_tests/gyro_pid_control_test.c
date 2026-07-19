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
#include "rgb_led.h"

typedef struct {
    float angle_integral;
    float angle_last_error;
    float rate_integral;
    float rate_last_error;
    bool derivative_ready;
} gyro_pid_controller_t;

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

static void gyro_pid_reset(gyro_pid_controller_t *pid)
{
    pid->angle_integral = 0.0f;
    pid->angle_last_error = 0.0f;
    pid->rate_integral = 0.0f;
    pid->rate_last_error = 0.0f;
    pid->derivative_ready = false;
}

static int gyro_pid_calculate(gyro_pid_controller_t *pid, float yaw_deg,
    float z_rate_dps, float *angle_error_out, float *target_rate_out)
{
    float angle_error = gyro_pid_wrap_angle(
        GYRO_PID_TARGET_YAW_DEG - yaw_deg);
    float angle_derivative = 0.0f;
    float rate_derivative = 0.0f;
    float target_rate;
    float rate_error;
    float turn_output;

    pid->angle_integral += angle_error;
    pid->angle_integral = gyro_pid_clamp_float(pid->angle_integral,
        -GYRO_PID_ANGLE_INTEGRAL_LIMIT, GYRO_PID_ANGLE_INTEGRAL_LIMIT);

    if (pid->derivative_ready) {
        angle_derivative = angle_error - pid->angle_last_error;
    }

    target_rate = (GYRO_PID_ANGLE_KP * angle_error) +
        (GYRO_PID_ANGLE_KI * pid->angle_integral) +
        (GYRO_PID_ANGLE_KD * angle_derivative);
    target_rate = gyro_pid_clamp_float(target_rate,
        -GYRO_PID_TARGET_RATE_LIMIT_DPS, GYRO_PID_TARGET_RATE_LIMIT_DPS);

    rate_error = target_rate - z_rate_dps;
    pid->rate_integral += rate_error;
    pid->rate_integral = gyro_pid_clamp_float(pid->rate_integral,
        -GYRO_PID_RATE_INTEGRAL_LIMIT, GYRO_PID_RATE_INTEGRAL_LIMIT);

    if (pid->derivative_ready) {
        rate_derivative = rate_error - pid->rate_last_error;
    }

    turn_output = ((GYRO_PID_RATE_KP * rate_error) +
        (GYRO_PID_RATE_KI * pid->rate_integral) +
        (GYRO_PID_RATE_KD * rate_derivative)) * GYRO_PID_OUTPUT_SCALE;
    turn_output *= GYRO_PID_TURN_DIRECTION;

    if (turn_output > 0.0f) {
        turn_output += (float) GYRO_PID_TURN_DEAD_ZONE_PWM;
    } else if (turn_output < 0.0f) {
        turn_output -= (float) GYRO_PID_TURN_DEAD_ZONE_PWM;
    }

    pid->angle_last_error = angle_error;
    pid->rate_last_error = rate_error;
    pid->derivative_ready = true;
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

    gyro_pid_display("WAIT MODULE 600ms", "UART3 RX = PB3", "MOTOR STOPPED");
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

    for (uint32_t seconds = GYRO_PID_START_DELAY_SECONDS;
         seconds > 0U; seconds--) {
        (void) snprintf(countdown_text, sizeof(countdown_text),
            "START IN %lu s", (unsigned long) seconds);
        gyro_pid_display("YAW ZERO READY", countdown_text, "RESET = STOP");
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
    led_set_color(COLOR_BLUE);

    if (!gyro_pid_find_baud_rate()) {
        led_set_color(COLOR_RED);
        gyro_pid_display("NO VALID FRAME", "CHECK TX -> PB3", "MOTOR STOPPED");
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
    gyro_pid_reset(&pid);

    for (;;) {
        uint32_t frame_count = lengke_gyro_get_valid_frame_count();
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
            turn_pwm = gyro_pid_calculate(&pid, yaw_deg, z_rate_dps,
                &angle_error, &target_rate);
            motor_pwms[0] = gyro_pid_clamp_pwm(
                GYRO_PID_BASE_PWM - turn_pwm);
            motor_pwms[1] = gyro_pid_clamp_pwm(
                GYRO_PID_BASE_PWM + turn_pwm);
            led_set_color(COLOR_GREEN);
        } else {
            motor_pwms[0] = 0;
            motor_pwms[1] = 0;
            gyro_pid_reset(&pid);
            led_set_color(COLOR_RED);
        }

        motor_set_pwms(motor_pwms);

        telemetry_counter++;
        if (telemetry_counter >= GYRO_PID_TELEMETRY_PERIOD_CYCLES) {
            telemetry_counter = 0U;
            /* VOFA+ FireWater: nine numeric channels, no text/header. */
            usart_printf(DEBUG_UART_INST,
                "%d,%d,%d,%d,%d,%d,%d,%d,%lu\n",
                gyro_pid_tenths(yaw_deg),
                gyro_pid_tenths(GYRO_PID_TARGET_YAW_DEG),
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
                "Y:", yaw_deg, "E:", angle_error);
            gyro_pid_format_pair(rate_text, sizeof(rate_text),
                "W:", z_rate_dps, "T:", target_rate);

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
