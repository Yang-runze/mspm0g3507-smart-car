#include "h24_figure_eight.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "board_led.h"
#include "delay.h"
#include "encoder_user.h"
#include "gray_detection.h"
#include "lengke_gyro.h"
#include "motor_user.h"
#include "oled_driver.h"
#include "systick.h"
#include "ui_button.h"

#define H24_PI_F                                      3.1415926f
#define H24_CENTER_SENSOR_PATTERN                           0x18U
#define H24_POINT_LED_FLASH_CYCLES \
    (H24_POINT_LED_FLASH_MS / H24_FIGURE_EIGHT_CONTROL_PERIOD_MS)

/* The existing button test identifies physical K1/K2 this way. */
#define H24_LAP_SELECT_BUTTON                      BUTTON_RIGHT
#define H24_START_BUTTON                            BUTTON_LEFT

typedef enum {
    H24_STATE_EXIT_A = 0,
    H24_STATE_TURN_TO_C,
    H24_STATE_STRAIGHT_A_TO_C,
    H24_STATE_ARC_C_TO_B,
    H24_STATE_TURN_TO_D,
    H24_STATE_STRAIGHT_B_TO_D,
    H24_STATE_ARC_D_TO_A
} h24_state_t;

typedef struct {
    float target_yaw_deg;
    float angle_integral_deg_s;
    float rate_integral_deg;
} h24_gyro_pid_t;

typedef struct {
    float integral;
    float last_error;
    float filtered_derivative;
    bool derivative_ready;
    bool has_line_history;
} h24_gray_pid_t;

typedef struct {
    float previous_wrapped_deg;
    float continuous_deg;
    bool initialized;
} h24_yaw_tracker_t;

typedef struct {
    h24_state_t state;
    h24_gyro_pid_t gyro_pid;
    h24_gray_pid_t gray_pid;
    h24_yaw_tracker_t yaw_tracker;
    int motor_pwms[2];
    int32_t state_left_count;
    int32_t state_right_count;
    float state_distance_mm;
    float arc_start_yaw_deg;
    float target_yaw_deg;
    float angle_error_deg;
    float target_rate_dps;
    float gray_error;
    int correction_pwm;
    uint32_t state_cycles;
    uint32_t line_confirm_cycles;
    uint32_t line_lost_cycles;
    uint32_t heading_settle_cycles;
    uint32_t led_flash_cycles;
    uint32_t last_gyro_frame_count;
    uint32_t stale_gyro_cycles;
    uint8_t sensors;
    uint8_t target_laps;
    uint8_t completed_laps;
} h24_context_t;

static float h24_clamp_float(float value, float minimum, float maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static float h24_abs_float(float value)
{
    return value < 0.0f ? -value : value;
}

static float h24_wrap_angle(float angle_deg)
{
    while (angle_deg > 180.0f) {
        angle_deg -= 360.0f;
    }
    while (angle_deg < -180.0f) {
        angle_deg += 360.0f;
    }
    return angle_deg;
}

static int h24_clamp_pwm(int value)
{
    if (value < 0) {
        return 0;
    }
    if (value > H24_DRIVE_MAX_PWM) {
        return H24_DRIVE_MAX_PWM;
    }
    return value;
}

static void h24_display(const char *line1, const char *line2,
    const char *line3)
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    u8g2_DrawStr(&u8g2, 0U, 12U, "H24 FIGURE EIGHT");
    u8g2_DrawStr(&u8g2, 0U, 28U, line1);
    u8g2_DrawStr(&u8g2, 0U, 42U, line2);
    u8g2_DrawStr(&u8g2, 0U, 56U, line3);
    u8g2_SendBuffer(&u8g2);
}

static void h24_format_float_pair(char *text, size_t text_size,
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

static const char *h24_state_name(h24_state_t state)
{
    switch (state) {
        case H24_STATE_EXIT_A:
            return "EXIT A";
        case H24_STATE_TURN_TO_C:
            return "TURN AC";
        case H24_STATE_STRAIGHT_A_TO_C:
            return "STRAIGHT AC";
        case H24_STATE_ARC_C_TO_B:
            return "ARC CB";
        case H24_STATE_TURN_TO_D:
            return "TURN BD";
        case H24_STATE_STRAIGHT_B_TO_D:
            return "STRAIGHT BD";
        case H24_STATE_ARC_D_TO_A:
            return "ARC DA";
        default:
            return "UNKNOWN";
    }
}

static void h24_fault(const char *message)
{
    const int stopped_pwms[2] = {0, 0};

    motor_set_pwms(stopped_pwms);
    h24_display("FAULT - STOPPED", message, "RESET AFTER CHECK");

    for (;;) {
        board_led_toggle();
        motor_set_pwms(stopped_pwms);
        delay_ms(100U);
    }
}

static void h24_complete(uint8_t completed_laps)
{
    const int stopped_pwms[2] = {0, 0};
    char lap_text[22];

    motor_set_pwms(stopped_pwms);
    (void) snprintf(lap_text, sizeof(lap_text), "LAPS COMPLETE:%u",
        (unsigned int) completed_laps);
    h24_display("STOPPED AT A", lap_text, "RESET TO RESTART");
    board_led_set(true);
    delay_ms(H24_POINT_LED_FLASH_MS);
    board_led_set(false);

    for (;;) {
        motor_set_pwms(stopped_pwms);
        delay_ms(100U);
    }
}

static bool h24_gyro_can_integrate(float unsaturated_output,
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

static void h24_gyro_reset(h24_gyro_pid_t *pid, float target_yaw_deg)
{
    pid->target_yaw_deg = h24_wrap_angle(target_yaw_deg);
    pid->angle_integral_deg_s = 0.0f;
    pid->rate_integral_deg = 0.0f;
}

static int h24_gyro_calculate(h24_gyro_pid_t *pid, float target_yaw_deg,
    float yaw_deg, float z_rate_dps,
    float *angle_error_out, float *target_rate_out)
{
    const float dt_s =
        (float) H24_FIGURE_EIGHT_CONTROL_PERIOD_MS / 1000.0f;
    float target_change = h24_wrap_angle(
        target_yaw_deg - pid->target_yaw_deg);
    float angle_error;
    float angle_integral_candidate;
    float target_rate_unsaturated;
    float target_rate;
    float rate_error;
    float rate_integral_candidate;
    float turn_output_unsaturated;
    float turn_output;

    if (h24_abs_float(target_change) >= 0.05f) {
        pid->angle_integral_deg_s = 0.0f;
        pid->rate_integral_deg = 0.0f;
    }
    pid->target_yaw_deg = h24_wrap_angle(target_yaw_deg);

    angle_error = h24_wrap_angle(pid->target_yaw_deg - yaw_deg);
    angle_integral_candidate = h24_clamp_float(
        pid->angle_integral_deg_s + (angle_error * dt_s),
        -GYRO_PID_ANGLE_INTEGRAL_LIMIT_DEG_S,
        GYRO_PID_ANGLE_INTEGRAL_LIMIT_DEG_S);
    target_rate_unsaturated =
        (GYRO_PID_ANGLE_KP * angle_error) +
        (GYRO_PID_ANGLE_KI * angle_integral_candidate);
    target_rate = h24_clamp_float(target_rate_unsaturated,
        -GYRO_PID_TARGET_RATE_LIMIT_DPS, GYRO_PID_TARGET_RATE_LIMIT_DPS);

    if (h24_gyro_can_integrate(
            target_rate_unsaturated, target_rate, angle_error)) {
        pid->angle_integral_deg_s = angle_integral_candidate;
    }

    rate_error = target_rate - z_rate_dps;
    rate_integral_candidate = h24_clamp_float(
        pid->rate_integral_deg + (rate_error * dt_s),
        -GYRO_PID_RATE_INTEGRAL_LIMIT_DEG,
        GYRO_PID_RATE_INTEGRAL_LIMIT_DEG);
    turn_output_unsaturated =
        (GYRO_PID_RATE_KP * rate_error) +
        (GYRO_PID_RATE_KI * rate_integral_candidate);
    turn_output = h24_clamp_float(turn_output_unsaturated,
        -(float) GYRO_PID_TURN_PWM_LIMIT,
        (float) GYRO_PID_TURN_PWM_LIMIT);

    if (h24_gyro_can_integrate(
            turn_output_unsaturated, turn_output, rate_error)) {
        pid->rate_integral_deg = rate_integral_candidate;
    }

    turn_output *= GYRO_PID_TURN_DIRECTION;
    *angle_error_out = angle_error;
    *target_rate_out = target_rate;
    return (int) turn_output;
}

static void h24_apply_gyro_heading(h24_context_t *context,
    float target_yaw_deg, float yaw_deg, float z_rate_dps, int base_pwm)
{
    int turn_pwm = h24_gyro_calculate(&context->gyro_pid,
        target_yaw_deg, yaw_deg, z_rate_dps,
        &context->angle_error_deg, &context->target_rate_dps);

    context->target_yaw_deg = target_yaw_deg;
    context->correction_pwm = turn_pwm;
    context->motor_pwms[0] = h24_clamp_pwm(base_pwm - turn_pwm);
    context->motor_pwms[1] = h24_clamp_pwm(base_pwm + turn_pwm);
}

static void h24_gray_reset(h24_gray_pid_t *pid)
{
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->filtered_derivative = 0.0f;
    pid->derivative_ready = false;
    pid->has_line_history = false;
}

static void h24_gray_reset_after_loss(h24_gray_pid_t *pid)
{
    pid->integral = 0.0f;
    pid->filtered_derivative = 0.0f;
    pid->derivative_ready = false;
}

static float h24_gray_calculate(h24_gray_pid_t *pid, float error)
{
    float derivative;
    float output;

    pid->integral += error;
    pid->integral = h24_clamp_float(pid->integral,
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
    output = h24_clamp_float(output,
        -GRAY_PID_OUTPUT_LIMIT, GRAY_PID_OUTPUT_LIMIT);

    pid->last_error = error;
    pid->has_line_history = true;
    return output;
}

static uint8_t h24_gray_to_physical_order(uint8_t raw_value)
{
    /* Auxiliary-board raw order is bit6..bit0,bit7 from left to right. */
    return (uint8_t) ((raw_value << 1U) | (raw_value >> 7U));
}

static bool h24_gray_get_position(uint8_t sensors, float *position)
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

static void h24_apply_gray_tracking(h24_context_t *context)
{
    float absolute_error = h24_abs_float(context->gray_error);
    int base_pwm = GRAY_PID_BASE_PWM -
        (int) (absolute_error * GRAY_PID_CURVE_SLOWDOWN_PER_UNIT);
    float correction = h24_gray_calculate(
        &context->gray_pid, context->gray_error);

    context->correction_pwm = (int) correction;
    context->target_yaw_deg = 0.0f;
    context->angle_error_deg = 0.0f;
    context->target_rate_dps = 0.0f;
    context->motor_pwms[0] = h24_clamp_pwm(
        base_pwm + context->correction_pwm);
    context->motor_pwms[1] = h24_clamp_pwm(
        base_pwm - context->correction_pwm);
}

static void h24_apply_arc_line_search(h24_context_t *context)
{
    int correction;

    if (!context->gray_pid.has_line_history) {
        context->motor_pwms[0] = 0;
        context->motor_pwms[1] = 0;
        context->correction_pwm = 0;
        return;
    }

    correction = context->gray_pid.last_error < 0.0f ?
        -GRAY_PID_SEARCH_CORRECTION_PWM :
        GRAY_PID_SEARCH_CORRECTION_PWM;
    context->correction_pwm = correction;
    context->motor_pwms[0] = h24_clamp_pwm(
        GRAY_PID_SEARCH_BASE_PWM + correction);
    context->motor_pwms[1] = h24_clamp_pwm(
        GRAY_PID_SEARCH_BASE_PWM - correction);
}

static void h24_yaw_tracker_init(h24_yaw_tracker_t *tracker,
    float wrapped_yaw_deg)
{
    tracker->previous_wrapped_deg = wrapped_yaw_deg;
    tracker->continuous_deg = wrapped_yaw_deg;
    tracker->initialized = true;
}

static void h24_yaw_tracker_update(h24_yaw_tracker_t *tracker,
    float wrapped_yaw_deg)
{
    float delta;

    if (!tracker->initialized) {
        h24_yaw_tracker_init(tracker, wrapped_yaw_deg);
        return;
    }

    /*
     * The module jumps between +179.9 and -179.9. Wrapping the sample-to-
     * sample delta converts that jump into the real small rotation.
     */
    delta = h24_wrap_angle(wrapped_yaw_deg -
        tracker->previous_wrapped_deg);
    tracker->continuous_deg += delta;
    tracker->previous_wrapped_deg = wrapped_yaw_deg;
}

static int32_t h24_forward_encoder_delta(int32_t current, int32_t start,
    bool inverted)
{
    int32_t delta = current - start;

    if (inverted) {
        delta = -delta;
    }
    return delta > 0 ? delta : 0;
}

static float h24_state_distance_mm(const h24_context_t *context)
{
    int32_t current_left = encoder_manager_read(
        &robot_encoder_manager, 0U);
    int32_t current_right = encoder_manager_read(
        &robot_encoder_manager, 1U);
    int32_t left_delta = h24_forward_encoder_delta(
        current_left, context->state_left_count,
        MOTOR_LEFT_ENCODER_INVERTED);
    int32_t right_delta = h24_forward_encoder_delta(
        current_right, context->state_right_count,
        MOTOR_RIGHT_ENCODER_INVERTED);
    float average_counts =
        ((float) left_delta + (float) right_delta) * 0.5f;

    return average_counts * (H24_PI_F * H24_WHEEL_DIAMETER_MM) /
        H24_ENCODER_COUNTS_PER_REV;
}

static void h24_start_led_flash(h24_context_t *context)
{
    context->led_flash_cycles = H24_POINT_LED_FLASH_CYCLES;
}

static void h24_update_led(h24_context_t *context)
{
    if (context->led_flash_cycles > 0U) {
        board_led_set(true);
        context->led_flash_cycles--;
    } else {
        board_led_set(false);
    }
}

static void h24_enter_state(h24_context_t *context, h24_state_t state)
{
    context->state = state;
    context->state_cycles = 0U;
    context->line_confirm_cycles = 0U;
    context->line_lost_cycles = 0U;
    context->heading_settle_cycles = 0U;
    context->state_left_count = encoder_manager_read(
        &robot_encoder_manager, 0U);
    context->state_right_count = encoder_manager_read(
        &robot_encoder_manager, 1U);
    context->state_distance_mm = 0.0f;
    context->arc_start_yaw_deg = context->yaw_tracker.continuous_deg;

    if (state == H24_STATE_EXIT_A) {
        context->target_yaw_deg = 0.0f;
        h24_gyro_reset(&context->gyro_pid, 0.0f);
    } else if (state == H24_STATE_TURN_TO_C ||
               state == H24_STATE_STRAIGHT_A_TO_C) {
        context->target_yaw_deg = H24_FIRST_DIAGONAL_YAW_DEG;
        h24_gyro_reset(&context->gyro_pid,
            H24_FIRST_DIAGONAL_YAW_DEG);
    } else if (state == H24_STATE_TURN_TO_D ||
               state == H24_STATE_STRAIGHT_B_TO_D) {
        context->target_yaw_deg = H24_SECOND_DIAGONAL_YAW_DEG;
        h24_gyro_reset(&context->gyro_pid,
            H24_SECOND_DIAGONAL_YAW_DEG);
    } else {
        h24_gray_reset(&context->gray_pid);
    }
}

static bool h24_find_gyro_baud_rate(void)
{
    static const uint32_t baud_rates[] = {
        115200U, 9600U, 4800U, 19200U,
        38400U, 57600U, 230400U, 2400U
    };
    char baud_text[22];

    h24_display("WAIT GYRO 600ms", "UART1 RX = PA9", "MOTOR STOPPED");
    delay_ms(600U);

    for (size_t index = 0U;
         index < sizeof(baud_rates) / sizeof(baud_rates[0]); index++) {
        lengke_gyro_set_host_baud_rate(baud_rates[index]);
        (void) snprintf(baud_text, sizeof(baud_text), "TRY BAUD:%lu",
            (unsigned long) baud_rates[index]);
        h24_display("SCANNING GYRO", baud_text, "MOTOR STOPPED");
        delay_ms(800U);

        if (lengke_gyro_has_valid_data()) {
            return true;
        }
    }

    return false;
}

static uint8_t h24_select_laps(void)
{
    uint8_t selected_laps = 0U;
    bool previous_k1 = false;
    bool previous_k2 = false;
    char lap_text[22];
    char sensor_text[22];
    char prompt_text[22];

    for (;;) {
        bool k1 = user_button_is_pressed(H24_LAP_SELECT_BUTTON);
        bool k2 = user_button_is_pressed(H24_START_BUTTON);
        uint8_t sensors = h24_gray_to_physical_order(
            (uint8_t) gray_read_byte());
        bool centered =
            sensors == H24_CENTER_SENSOR_PATTERN;

        if (k1 && !previous_k1) {
            selected_laps++;
            if (selected_laps > 4U) {
                selected_laps = 1U;
            }
        }

        if (k2 && !previous_k2 && selected_laps > 0U && centered) {
            board_led_set(false);
            return selected_laps;
        }

        (void) snprintf(lap_text, sizeof(lap_text), "K1 LAPS:%u",
            (unsigned int) selected_laps);
        (void) snprintf(sensor_text, sizeof(sensor_text), "S:%02X NEED:18",
            (unsigned int) sensors);
        if (selected_laps == 0U) {
            (void) snprintf(prompt_text, sizeof(prompt_text),
                "PRESS K1 TO SELECT");
        } else if (!centered) {
            (void) snprintf(prompt_text, sizeof(prompt_text),
                "CENTER SENSOR ON A");
        } else {
            (void) snprintf(prompt_text, sizeof(prompt_text),
                "K2 START");
        }
        h24_display(lap_text, sensor_text, prompt_text);

        previous_k1 = k1;
        previous_k2 = k2;
        delay_ms(20U);
    }
}

static void h24_zero_and_countdown(uint8_t selected_laps)
{
    char countdown_text[22];
    char lap_text[22];

    h24_display("GYRO ZERO", "KEEP CAR STILL", "FACING A -> B");
    sendCaliYawCommand();
    delay_ms(500U);

    (void) snprintf(lap_text, sizeof(lap_text), "LAPS:%u",
        (unsigned int) selected_laps);
    for (uint32_t seconds = H24_FIGURE_EIGHT_START_DELAY_SECONDS;
         seconds > 0U; seconds--) {
        (void) snprintf(countdown_text, sizeof(countdown_text),
            "START IN %lu s", (unsigned long) seconds);
        h24_display(lap_text, countdown_text, "FACING A -> B");
        delay_ms(1000U);
    }
}

static void h24_update_run_display(const h24_context_t *context,
    float yaw_deg)
{
    char state_text[22];
    char yaw_text[22];
    char sensor_text[22];

    (void) snprintf(state_text, sizeof(state_text), "%s %u/%u",
        h24_state_name(context->state),
        (unsigned int) context->completed_laps,
        (unsigned int) context->target_laps);
    h24_format_float_pair(yaw_text, sizeof(yaw_text),
        "Y:", yaw_deg, "T:", context->target_yaw_deg);
    (void) snprintf(sensor_text, sizeof(sensor_text), "S:%02X D:%d",
        (unsigned int) context->sensors,
        (int) context->state_distance_mm);
    h24_display(state_text, yaw_text, sensor_text);
}

void h24_figure_eight_run(void)
{
    h24_context_t context = {0};
    uint32_t display_cycles = 0U;

    u8g2_Init();
    systick_init();
    encoder_application_init();
    gray_detection_init();
    motor_init();
    motor_set_pwms(context.motor_pwms);
    lengke_gyro_init();
    board_led_set(false);

    if (!h24_find_gyro_baud_rate()) {
        h24_fault("NO VALID GYRO");
    }

    context.target_laps = h24_select_laps();
    h24_zero_and_countdown(context.target_laps);

    encoder_manager_write(&robot_encoder_manager, 0U, 0);
    encoder_manager_write(&robot_encoder_manager, 1U, 0);
    h24_yaw_tracker_init(&context.yaw_tracker, Yaw());
    context.last_gyro_frame_count =
        lengke_gyro_get_valid_frame_count();
    h24_enter_state(&context, H24_STATE_EXIT_A);
    h24_start_led_flash(&context);

    for (;;) {
        uint32_t gyro_frame_count =
            lengke_gyro_get_valid_frame_count();
        float yaw_deg = Yaw();
        float z_rate_dps = GyroZ();
        bool line_found;

        h24_yaw_tracker_update(&context.yaw_tracker, yaw_deg);
        context.sensors = h24_gray_to_physical_order(
            (uint8_t) gray_read_byte());
        line_found = h24_gray_get_position(
            context.sensors, &context.gray_error);
        context.state_distance_mm = h24_state_distance_mm(&context);
        context.state_cycles++;

        if (gyro_frame_count != context.last_gyro_frame_count) {
            context.last_gyro_frame_count = gyro_frame_count;
            context.stale_gyro_cycles = 0U;
        } else if (context.stale_gyro_cycles <
                   H24_GYRO_DATA_TIMEOUT_CYCLES) {
            context.stale_gyro_cycles++;
        }
        if (context.stale_gyro_cycles >=
            H24_GYRO_DATA_TIMEOUT_CYCLES) {
            h24_fault("GYRO DATA LOST");
        }

        switch (context.state) {
            case H24_STATE_EXIT_A:
                h24_apply_gyro_heading(&context, 0.0f,
                    yaw_deg, z_rate_dps, H24_START_EXIT_PWM);

                if (context.sensors == 0U) {
                    context.line_confirm_cycles++;
                } else {
                    context.line_confirm_cycles = 0U;
                }

                if (context.line_confirm_cycles >=
                    H24_LINE_CONFIRM_CYCLES) {
                    h24_enter_state(&context, H24_STATE_TURN_TO_C);
                } else if (context.state_distance_mm >
                           H24_START_EXIT_MAX_DISTANCE_MM) {
                    h24_fault("A LINE NOT LEFT");
                }
                break;

            case H24_STATE_TURN_TO_C:
                h24_apply_gyro_heading(&context,
                    H24_FIRST_DIAGONAL_YAW_DEG,
                    yaw_deg, z_rate_dps,
                    H24_HEADING_ACQUIRE_BASE_PWM);

                if (h24_abs_float(context.angle_error_deg) <=
                        H24_HEADING_TOLERANCE_DEG &&
                    h24_abs_float(z_rate_dps) <=
                        H24_HEADING_RATE_TOLERANCE_DPS) {
                    context.heading_settle_cycles++;
                } else {
                    context.heading_settle_cycles = 0U;
                }

                if (context.heading_settle_cycles >=
                    H24_HEADING_SETTLE_CYCLES) {
                    h24_enter_state(&context,
                        H24_STATE_STRAIGHT_A_TO_C);
                } else if (context.state_cycles >=
                           H24_HEADING_ACQUIRE_TIMEOUT_CYCLES) {
                    h24_fault("TURN AC TIMEOUT");
                }
                break;

            case H24_STATE_STRAIGHT_A_TO_C:
                h24_apply_gyro_heading(&context,
                    H24_FIRST_DIAGONAL_YAW_DEG,
                    yaw_deg, z_rate_dps, GYRO_PID_BASE_PWM);

                if (context.state_distance_mm >=
                        H24_DIAGONAL_LINE_ENABLE_DISTANCE_MM &&
                    line_found) {
                    context.line_confirm_cycles++;
                } else if (!line_found) {
                    context.line_confirm_cycles = 0U;
                }

                if (context.line_confirm_cycles >=
                    H24_LINE_CONFIRM_CYCLES) {
                    h24_start_led_flash(&context); /* Point C. */
                    h24_enter_state(&context, H24_STATE_ARC_C_TO_B);
                } else if (context.state_distance_mm >
                           H24_DIAGONAL_MAX_DISTANCE_MM) {
                    h24_fault("POINT C NOT FOUND");
                }
                break;

            case H24_STATE_ARC_C_TO_B:
            {
                float arc_yaw = h24_abs_float(
                    context.yaw_tracker.continuous_deg -
                    context.arc_start_yaw_deg);
                bool endpoint_enabled =
                    context.state_distance_mm >=
                        H24_ARC_END_ENABLE_DISTANCE_MM &&
                    arc_yaw >= H24_ARC_END_ENABLE_YAW_DEG;

                if (line_found) {
                    context.line_lost_cycles = 0U;
                    h24_apply_gray_tracking(&context);
                } else {
                    context.line_lost_cycles++;
                    h24_gray_reset_after_loss(&context.gray_pid);
                    h24_apply_arc_line_search(&context);
                }

                if (endpoint_enabled &&
                    context.line_lost_cycles >=
                        H24_LINE_CONFIRM_CYCLES) {
                    h24_start_led_flash(&context); /* Point B. */
                    h24_enter_state(&context, H24_STATE_TURN_TO_D);
                } else if ((!endpoint_enabled &&
                            context.line_lost_cycles >
                                H24_ARC_LINE_RECOVERY_CYCLES) ||
                           context.state_distance_mm >
                                H24_ARC_MAX_DISTANCE_MM) {
                    h24_fault("ARC CB LINE LOST");
                }
                break;
            }

            case H24_STATE_TURN_TO_D:
                h24_apply_gyro_heading(&context,
                    H24_SECOND_DIAGONAL_YAW_DEG,
                    yaw_deg, z_rate_dps,
                    H24_HEADING_ACQUIRE_BASE_PWM);

                if (h24_abs_float(context.angle_error_deg) <=
                        H24_HEADING_TOLERANCE_DEG &&
                    h24_abs_float(z_rate_dps) <=
                        H24_HEADING_RATE_TOLERANCE_DPS) {
                    context.heading_settle_cycles++;
                } else {
                    context.heading_settle_cycles = 0U;
                }

                if (context.heading_settle_cycles >=
                    H24_HEADING_SETTLE_CYCLES) {
                    h24_enter_state(&context,
                        H24_STATE_STRAIGHT_B_TO_D);
                } else if (context.state_cycles >=
                           H24_HEADING_ACQUIRE_TIMEOUT_CYCLES) {
                    h24_fault("TURN BD TIMEOUT");
                }
                break;

            case H24_STATE_STRAIGHT_B_TO_D:
                h24_apply_gyro_heading(&context,
                    H24_SECOND_DIAGONAL_YAW_DEG,
                    yaw_deg, z_rate_dps, GYRO_PID_BASE_PWM);

                if (context.state_distance_mm >=
                        H24_DIAGONAL_LINE_ENABLE_DISTANCE_MM &&
                    line_found) {
                    context.line_confirm_cycles++;
                } else if (!line_found) {
                    context.line_confirm_cycles = 0U;
                }

                if (context.line_confirm_cycles >=
                    H24_LINE_CONFIRM_CYCLES) {
                    h24_start_led_flash(&context); /* Point D. */
                    h24_enter_state(&context, H24_STATE_ARC_D_TO_A);
                } else if (context.state_distance_mm >
                           H24_DIAGONAL_MAX_DISTANCE_MM) {
                    h24_fault("POINT D NOT FOUND");
                }
                break;

            case H24_STATE_ARC_D_TO_A:
            {
                float arc_yaw = h24_abs_float(
                    context.yaw_tracker.continuous_deg -
                    context.arc_start_yaw_deg);
                bool endpoint_enabled =
                    context.state_distance_mm >=
                        H24_ARC_END_ENABLE_DISTANCE_MM &&
                    arc_yaw >= H24_ARC_END_ENABLE_YAW_DEG;

                if (line_found) {
                    context.line_lost_cycles = 0U;
                    h24_apply_gray_tracking(&context);
                } else {
                    context.line_lost_cycles++;
                    h24_gray_reset_after_loss(&context.gray_pid);
                    h24_apply_arc_line_search(&context);
                }

                if (endpoint_enabled &&
                    context.line_lost_cycles >=
                        H24_LINE_CONFIRM_CYCLES) {
                    context.completed_laps++;
                    h24_start_led_flash(&context); /* Point A. */

                    if (context.completed_laps >= context.target_laps) {
                        h24_complete(context.completed_laps);
                    }

                    h24_enter_state(&context, H24_STATE_TURN_TO_C);
                } else if ((!endpoint_enabled &&
                            context.line_lost_cycles >
                                H24_ARC_LINE_RECOVERY_CYCLES) ||
                           context.state_distance_mm >
                                H24_ARC_MAX_DISTANCE_MM) {
                    h24_fault("ARC DA LINE LOST");
                }
                break;
            }

            default:
                h24_fault("INVALID STATE");
                break;
        }

        motor_set_pwms(context.motor_pwms);
        h24_update_led(&context);

        display_cycles++;
        if (display_cycles >=
            H24_FIGURE_EIGHT_DISPLAY_PERIOD_CYCLES) {
            display_cycles = 0U;
            h24_update_run_display(&context, yaw_deg);
        }

        delay_ms(H24_FIGURE_EIGHT_CONTROL_PERIOD_MS);
    }
}
