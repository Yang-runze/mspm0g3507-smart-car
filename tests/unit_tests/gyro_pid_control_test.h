#ifndef GYRO_PID_CONTROL_TEST_H
#define GYRO_PID_CONTROL_TEST_H

#include "motor_config.h"

/*
 * Gyro cascaded PID tuning area.
 *
 * The angle/rate gains below come from the manufacturer's GyroPID example:
 * yaw angle PID -> target Z rate -> Z-rate PID -> differential motor PWM.
 */
#define GYRO_PID_ANGLE_KP                       6.55f
#define GYRO_PID_ANGLE_KI                       0.03f
#define GYRO_PID_ANGLE_KD                       0.00f
#define GYRO_PID_ANGLE_INTEGRAL_LIMIT         300.0f

#define GYRO_PID_RATE_KP                        0.450f
#define GYRO_PID_RATE_KI                        0.025f
#define GYRO_PID_RATE_KD                        0.000f
#define GYRO_PID_RATE_INTEGRAL_LIMIT           80.0f

/* Scale the manufacturer's approximately +/-200 output to this car's PWM. */
#define GYRO_PID_OUTPUT_SCALE                    6.0f
#define GYRO_PID_TURN_DEAD_ZONE_PWM               96
#define GYRO_PID_TARGET_RATE_LIMIT_DPS          180.0f

/* For in-place heading hold, set BASE_PWM to 0 and enable motor reverse. */
#define GYRO_PID_TARGET_YAW_DEG                   0.0f
#define GYRO_PID_BASE_PWM                         700
#define GYRO_PID_MAX_PWM                         1400
/* Keep 0 for the straight-line test; set 1 only after safe reversal testing. */
#define GYRO_PID_ALLOW_MOTOR_REVERSE                0

/* Change to -1.0f if steering correction runs in the opposite direction. */
#define GYRO_PID_TURN_DIRECTION                   1.0f

#define GYRO_PID_CONTROL_PERIOD_MS                 10U
#define GYRO_PID_DISPLAY_PERIOD_CYCLES             10U
#define GYRO_PID_TELEMETRY_PERIOD_CYCLES            5U
#define GYRO_PID_DATA_TIMEOUT_CYCLES               30U
#define GYRO_PID_START_DELAY_SECONDS                3U

#if GYRO_PID_MAX_PWM > MOTOR_PWM_COMMAND_MAX
#error "GYRO_PID_MAX_PWM exceeds the configured motor PWM period"
#endif

void gyro_pid_control_test_run(void);

#endif
