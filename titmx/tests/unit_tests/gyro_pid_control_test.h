#ifndef GYRO_PID_CONTROL_TEST_H
#define GYRO_PID_CONTROL_TEST_H

#include "motor_config.h"

/*
 * Gyro heading-hold cascade controller.
 *
 * Runtime target yaw -> angle PI -> target Z rate -> rate PI
 *                    -> differential motor PWM.
 *
 * The defaults are conservative bring-up values for the 12 V, 1:30
 * XTARK MC520P30 motors and this project's 0...3000 PWM command range.
 * They are starting values only; tune them on the complete vehicle.
 */
#define GYRO_PID_ANGLE_KP                         5.0f
#define GYRO_PID_ANGLE_KI                         0.0f
/* Integral state unit: degree * second. Keep KI at zero for initial tuning. */
#define GYRO_PID_ANGLE_INTEGRAL_LIMIT_DEG_S      20.0f

/* Rate-loop units: KP = PWM/(deg/s), KI = PWM/degree. */
#define GYRO_PID_RATE_KP                          4.0f
#define GYRO_PID_RATE_KI                         12.0f
/* Integral state unit: degree; KI * this value is integral-output PWM. */
#define GYRO_PID_RATE_INTEGRAL_LIMIT_DEG         15.0f

#define GYRO_PID_TARGET_RATE_LIMIT_DPS            90.0f
#define GYRO_PID_TURN_PWM_LIMIT                     600

/*
 * The module yaw is zeroed before the countdown. Change this value to make the
 * standalone test acquire another absolute heading, or call
 * gyro_pid_set_target_yaw() later from application/command code.
 */
#define GYRO_PID_INITIAL_TARGET_YAW_DEG             0.0f
/*
 * Use a higher open-loop cruise command so this test has a speed closer to the
 * 100...120 RPM motor-PID response test. With the 600 PWM turn limit, the
 * commanded wheel range remains forward-only at 600...1800.
 */
#define GYRO_PID_BASE_PWM                        1200
#define GYRO_PID_MAX_PWM                         1800
/* Heading-hold driving defaults to forward-only motor commands. */
#define GYRO_PID_ALLOW_MOTOR_REVERSE                0

/* Change to -1.0f if steering correction runs in the opposite direction. */
#define GYRO_PID_TURN_DIRECTION                   -1.0f

#define GYRO_PID_CONTROL_PERIOD_MS                 10U
#define GYRO_PID_DISPLAY_PERIOD_CYCLES             10U
#define GYRO_PID_TELEMETRY_PERIOD_CYCLES            5U
#define GYRO_PID_DATA_TIMEOUT_CYCLES               30U
#define GYRO_PID_START_DELAY_SECONDS                3U

#if GYRO_PID_MAX_PWM > MOTOR_PWM_COMMAND_MAX
#error "GYRO_PID_MAX_PWM exceeds the configured motor PWM period"
#endif

#if GYRO_PID_BASE_PWM > GYRO_PID_MAX_PWM
#error "GYRO_PID_BASE_PWM exceeds GYRO_PID_MAX_PWM"
#endif

#if !GYRO_PID_ALLOW_MOTOR_REVERSE && \
    (GYRO_PID_TURN_PWM_LIMIT > GYRO_PID_BASE_PWM)
#error "Forward-only turn limit would command motor reverse"
#endif

#if (GYRO_PID_BASE_PWM + GYRO_PID_TURN_PWM_LIMIT) > GYRO_PID_MAX_PWM
#error "Base PWM plus turn limit exceeds GYRO_PID_MAX_PWM"
#endif

void gyro_pid_set_target_yaw(float target_yaw_deg);
float gyro_pid_get_target_yaw(void);
void gyro_pid_control_test_run(void);

#endif
