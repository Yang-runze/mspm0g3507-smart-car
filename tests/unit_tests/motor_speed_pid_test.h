#ifndef MOTOR_SPEED_PID_TEST_H
#define MOTOR_SPEED_PID_TEST_H

#include "motor_config.h"

/*
 * Independent two-wheel speed PID tuning area.
 * Feedback unit is wheel RPM, calculated from encoder counts and real dt.
 */
#define MOTOR_SPEED_PID_LEFT_KP                    8.0f
#define MOTOR_SPEED_PID_LEFT_KI                   18.0f
#define MOTOR_SPEED_PID_LEFT_KD                    0.0f
#define MOTOR_SPEED_PID_RIGHT_KP                   8.0f
#define MOTOR_SPEED_PID_RIGHT_KI                  18.0f
#define MOTOR_SPEED_PID_RIGHT_KD                   0.0f

#define MOTOR_SPEED_PID_TARGET_RPM               100.0f
/* MC520P30: 30:1 * 13 cycles/channel * current driver's x4 decode. */
#define MOTOR_SPEED_PID_ENCODER_COUNTS_PER_REV \
    ((float) MOTOR_ENCODER_COUNTS_PER_OUTPUT_REV)
#define MOTOR_SPEED_PID_FILTER_KEEP                0.55f
#define MOTOR_SPEED_PID_INTEGRAL_OUTPUT_LIMIT    800.0f
#define MOTOR_SPEED_PID_DERIVATIVE_LIMIT        1000.0f
#define MOTOR_SPEED_PID_MAX_PWM                   1600
#define MOTOR_SPEED_PID_DEAD_ZONE_PWM              100

/* Current encoder_user.c mapping: encoder 0 = left, encoder 1 = right. */
#define MOTOR_SPEED_PID_LEFT_ENCODER_INDEX           0U
#define MOTOR_SPEED_PID_RIGHT_ENCODER_INDEX          1U

/* Change either sign to -1.0f if that wheel reports negative RPM forward. */
#define MOTOR_SPEED_PID_LEFT_ENCODER_SIGN           1.0f
#define MOTOR_SPEED_PID_RIGHT_ENCODER_SIGN          1.0f

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
