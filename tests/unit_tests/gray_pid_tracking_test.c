#include "gray_pid_tracking_test.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "delay.h"
#include "gray_detection.h"
#include "hal_uart.h"
#include "motor_user.h"
#include "oled_driver.h"
#include "rgb_led.h"

typedef struct {
    float integral;
    float last_error;
    float filtered_derivative;
    bool derivative_ready;
    bool has_line_history;
} gray_pid_controller_t;

typedef struct {
    int previous_command;
    uint32_t kick_cycles_remaining;
} gray_pid_motor_start_t;

#define GRAY_PID_START_KICK_CYCLES \
    (GRAY_PID_START_KICK_MS / GRAY_PID_CONTROL_PERIOD_MS)

static float gray_pid_clamp_float(float value, float minimum, float maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static int gray_pid_clamp_pwm(int value)
{
    if (value < 0) {
        return 0;
    }
    if (value > GRAY_PID_MAX_PWM) {
        return GRAY_PID_MAX_PWM;
    }
    return value;
}

static int gray_pid_apply_start_kick(gray_pid_motor_start_t *start,
    int command)
{
    int applied_command = command;

    if (command <= 0) {
        start->previous_command = 0;
        start->kick_cycles_remaining = 0U;
        return 0;
    }

    if (start->previous_command <= 0) {
        start->kick_cycles_remaining = GRAY_PID_START_KICK_CYCLES;
    }

    if (start->kick_cycles_remaining > 0U) {
        if (applied_command < GRAY_PID_START_KICK_PWM) {
            applied_command = GRAY_PID_START_KICK_PWM;
        }
        start->kick_cycles_remaining--;
    }

    start->previous_command = command;
    return applied_command;
}

static uint8_t gray_pid_to_physical_order(uint8_t raw_value)
{
    /* Auxiliary-board raw order is bit6..bit0,bit7 from left to right. */
    return (uint8_t) ((raw_value << 1U) | (raw_value >> 7U));
}

static bool gray_pid_get_position(uint8_t sensors, float *position)
{
    int32_t weighted_sum = 0;
    uint8_t active_count = 0U;

    for (uint8_t index = 0U; index < 8U; index++) {
        uint8_t mask = (uint8_t) (0x80U >> index);

        if ((sensors & mask) != 0U) {
            weighted_sum += -7 + ((int32_t) index * 2);
            active_count++;
        }
    }

    if (active_count == 0U) {
        return false;
    }

    *position = (float) weighted_sum / (float) active_count;
    return true;
}

static float gray_pid_calculate(gray_pid_controller_t *pid, float error)
{
    float derivative;
    float output;

    pid->integral += error;
    pid->integral = gray_pid_clamp_float(pid->integral,
        -GRAY_PID_INTEGRAL_LIMIT, GRAY_PID_INTEGRAL_LIMIT);

    if (!pid->derivative_ready) {
        derivative = 0.0f;
        pid->derivative_ready = true;
    } else {
        derivative = error - pid->last_error;
    }

    pid->filtered_derivative =
        (GRAY_PID_D_FILTER_KEEP * pid->filtered_derivative) +
        ((1.0f - GRAY_PID_D_FILTER_KEEP) * derivative);

    output = (GRAY_PID_KP * error) +
        (GRAY_PID_KI * pid->integral) +
        (GRAY_PID_KD * pid->filtered_derivative);
    output = gray_pid_clamp_float(output,
        -GRAY_PID_OUTPUT_LIMIT, GRAY_PID_OUTPUT_LIMIT);

    pid->last_error = error;
    pid->has_line_history = true;
    return output;
}

static void gray_pid_reset_after_line_loss(gray_pid_controller_t *pid)
{
    pid->integral = 0.0f;
    pid->filtered_derivative = 0.0f;
    pid->derivative_ready = false;
}

static void gray_pid_format_bits(char *text, size_t text_size, uint8_t sensors)
{
    (void) snprintf(text, text_size, "S:%c%c%c%c%c%c%c%c",
        (sensors & 0x80U) ? '1' : '0', (sensors & 0x40U) ? '1' : '0',
        (sensors & 0x20U) ? '1' : '0', (sensors & 0x10U) ? '1' : '0',
        (sensors & 0x08U) ? '1' : '0', (sensors & 0x04U) ? '1' : '0',
        (sensors & 0x02U) ? '1' : '0', (sensors & 0x01U) ? '1' : '0');
}

static void gray_pid_format_control(char *text, size_t text_size,
    float error, int correction)
{
    int error_tenths = (int) (error * 10.0f);
    int error_magnitude = error_tenths < 0 ? -error_tenths : error_tenths;

    (void) snprintf(text, text_size, "E:%s%d.%d U:%d",
        error_tenths < 0 ? "-" : "", error_magnitude / 10,
        error_magnitude % 10, correction);
}

static void gray_pid_display(const char *line1, const char *line2,
    const char *line3)
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    u8g2_DrawStr(&u8g2, 0U, 12U, "GRAY PID TRACK");
    u8g2_DrawStr(&u8g2, 0U, 28U, line1);
    u8g2_DrawStr(&u8g2, 0U, 42U, line2);
    u8g2_DrawStr(&u8g2, 0U, 56U, line3);
    u8g2_SendBuffer(&u8g2);
}

static void gray_pid_start_countdown(void)
{
    char countdown_text[22];

    for (uint32_t seconds = GRAY_PID_START_DELAY_SECONDS;
         seconds > 0U; seconds--) {
        (void) snprintf(countdown_text, sizeof(countdown_text),
            "START IN %lu s", (unsigned long) seconds);
        gray_pid_display("PLACE ON TRACK", countdown_text,
            "RESET = STOP");
        delay_ms(1000U);
    }
}

void gray_pid_tracking_test_run(void)
{
    gray_pid_controller_t pid = {0};
    gray_pid_motor_start_t motor_start[2] = {0};
    int motor_pwms[2] = {0, 0};
    uint32_t display_counter = 0U;
    uint32_t telemetry_counter = 0U;
    uint32_t lost_cycles = 0U;
    char sensor_text[22];
    char control_text[22];
    char motor_text[22];

    u8g2_Init();
    gray_detection_init();
    motor_init();
    motor_set_pwms(motor_pwms);

    led_set_color(COLOR_BLUE);
    gray_pid_start_countdown();

    for (;;) {
        uint8_t raw_value = (uint8_t) gray_read_byte();
        uint8_t sensors = gray_pid_to_physical_order(raw_value);
        float error = 0.0f;
        float correction = 0.0f;
        bool line_found = gray_pid_get_position(sensors, &error);

        if (line_found) {
            float absolute_error = error < 0.0f ? -error : error;
            int base_pwm = GRAY_PID_BASE_PWM -
                (int) (absolute_error * GRAY_PID_CURVE_SLOWDOWN_PER_UNIT);

            correction = gray_pid_calculate(&pid, error);
            motor_pwms[0] = gray_pid_clamp_pwm(base_pwm + (int) correction);
            motor_pwms[1] = gray_pid_clamp_pwm(base_pwm - (int) correction);
            lost_cycles = 0U;
            led_set_color(COLOR_GREEN);
        } else {
            lost_cycles++;
            gray_pid_reset_after_line_loss(&pid);

            if (pid.has_line_history &&
                lost_cycles <= GRAY_PID_LOST_SEARCH_CYCLES) {
                correction = pid.last_error < 0.0f ?
                    -GRAY_PID_SEARCH_CORRECTION_PWM :
                    GRAY_PID_SEARCH_CORRECTION_PWM;
                motor_pwms[0] = gray_pid_clamp_pwm(
                    GRAY_PID_SEARCH_BASE_PWM + (int) correction);
                motor_pwms[1] = gray_pid_clamp_pwm(
                    GRAY_PID_SEARCH_BASE_PWM - (int) correction);
            } else {
                motor_pwms[0] = 0;
                motor_pwms[1] = 0;
            }
            led_set_color(COLOR_RED);
        }

        motor_pwms[0] = gray_pid_apply_start_kick(&motor_start[0],
            motor_pwms[0]);
        motor_pwms[1] = gray_pid_apply_start_kick(&motor_start[1],
            motor_pwms[1]);
        motor_set_pwms(motor_pwms);

        telemetry_counter++;
        if (telemetry_counter >= GRAY_PID_TELEMETRY_PERIOD_CYCLES) {
            telemetry_counter = 0U;
            /* VOFA+ FireWater: six numeric channels, no text/header. */
            usart_printf(DEBUG_UART_INST, "%u,%d,%d,%d,%d,%u\n",
                (unsigned int) sensors, (int) (error * 10.0f),
                (int) correction, motor_pwms[0], motor_pwms[1],
                line_found ? 1U : 0U);
        }

        display_counter++;
        if (display_counter >= GRAY_PID_DISPLAY_PERIOD_CYCLES) {
            display_counter = 0U;
            gray_pid_format_bits(sensor_text, sizeof(sensor_text), sensors);

            if (line_found) {
                gray_pid_format_control(control_text, sizeof(control_text),
                    error, (int) correction);
            } else if (motor_pwms[0] != 0 || motor_pwms[1] != 0) {
                (void) snprintf(control_text, sizeof(control_text),
                    "LOST: SEARCHING");
            } else {
                (void) snprintf(control_text, sizeof(control_text),
                    "LOST: STOPPED");
            }

            (void) snprintf(motor_text, sizeof(motor_text), "L:%d R:%d",
                motor_pwms[0], motor_pwms[1]);
            gray_pid_display(sensor_text, control_text, motor_text);
        }

        delay_ms(GRAY_PID_CONTROL_PERIOD_MS);
    }
}
