#ifndef MOTOR_SPEED_PID_TEST_H
#define MOTOR_SPEED_PID_TEST_H

#include "motor_config.h"

/*
 * Independent two-wheel speed PID tuning area.
 * Feedback unit is wheel RPM, calculated from encoder counts and real dt.
 */
#define MOTOR_SPEED_PID_LEFT_KP                    6.0f
#define MOTOR_SPEED_PID_LEFT_KI                   25.0f
#define MOTOR_SPEED_PID_LEFT_KD                    0.0f
#define MOTOR_SPEED_PID_RIGHT_KP                   6.0f
#define MOTOR_SPEED_PID_RIGHT_KI                  25.0f
#define MOTOR_SPEED_PID_RIGHT_KD                   0.0f

/* First target in the repeating 100 -> 50 -> 120 RPM response test. */
#define MOTOR_SPEED_PID_TARGET_RPM               100.0f
#define MOTOR_SPEED_PID_STEP_2_RPM                50.0f
#define MOTOR_SPEED_PID_STEP_3_RPM               120.0f
#define MOTOR_SPEED_PID_STEP_HOLD_MS              5000U
#define MOTOR_SPEED_PID_ENABLE_TARGET_PROFILE        1

/*
 * 3000 PWM / 360 no-load RPM is 8.33 PWM/RPM. Start slightly lower so the
 * PI loop supplies the remaining load-dependent PWM without a large overshoot.
 */
#define MOTOR_SPEED_PID_FEEDFORWARD_PWM_PER_RPM     8.0f
/* MC520P30 Hall version: 30:1 * 13 cycles/channel * x4 decode = 1560. */
#define MOTOR_SPEED_PID_ENCODER_COUNTS_PER_REV \
    ((float) MOTOR_ENCODER_COUNTS_PER_OUTPUT_REV)
#define MOTOR_SPEED_PID_FILTER_KEEP                0.55f
#define MOTOR_SPEED_PID_INTEGRAL_OUTPUT_LIMIT   1500.0f
#define MOTOR_SPEED_PID_DERIVATIVE_LIMIT        1000.0f
#define MOTOR_SPEED_PID_MAX_PWM                   1600
#define MOTOR_SPEED_PID_DEAD_ZONE_PWM              100

/* Current encoder_user.c mapping: encoder 0 = left, encoder 1 = right. */
#define MOTOR_SPEED_PID_LEFT_ENCODER_INDEX           0U
#define MOTOR_SPEED_PID_RIGHT_ENCODER_INDEX          1U

/* Reuse the installation calibration shared by all encoder-based tests. */
#define MOTOR_SPEED_PID_LEFT_ENCODER_SIGN \
    (MOTOR_LEFT_ENCODER_INVERTED ? -1.0f : 1.0f)
#define MOTOR_SPEED_PID_RIGHT_ENCODER_SIGN \
    (MOTOR_RIGHT_ENCODER_INVERTED ? -1.0f : 1.0f)

#define MOTOR_SPEED_PID_CONTROL_PERIOD_MS            20U
#define MOTOR_SPEED_PID_TELEMETRY_PERIOD_CYCLES       2U
#define MOTOR_SPEED_PID_DISPLAY_PERIOD_CYCLES         5U
#define MOTOR_SPEED_PID_START_DELAY_SECONDS           3U
#define MOTOR_SPEED_PID_WRONG_SIGN_RPM                5.0f
#define MOTOR_SPEED_PID_WRONG_SIGN_CYCLES              5U
#define MOTOR_SPEED_PID_STALL_RPM                     2.0f
#define MOTOR_SPEED_PID_STALL_PWM                      700
#define MOTOR_SPEED_PID_STALL_CYCLES                   25U

#if MOTOR_SPEED_PID_MAX_PWM > MOTOR_PWM_COMMAND_MAX
#error "MOTOR_SPEED_PID_MAX_PWM exceeds the configured motor PWM period"
#endif

void motor_speed_pid_test_run(void);

#endif
