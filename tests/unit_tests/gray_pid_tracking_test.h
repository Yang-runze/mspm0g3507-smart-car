#ifndef GRAY_PID_TRACKING_TEST_H
#define GRAY_PID_TRACKING_TEST_H

#include "motor_config.h"

/*
 * Gray tracking PID tuning area.
 *
 * Sensor position is normalized to -7 ... +7 from physical left to right.
 * MC520P30 motor PWM range is 0 ... MOTOR_PWM_COMMAND_MAX.
 */
#define GRAY_PID_KP                         100.0f
#define GRAY_PID_KI                           0.8f
#define GRAY_PID_KD                         160.0f
#define GRAY_PID_INTEGRAL_LIMIT              20.0f
#define GRAY_PID_OUTPUT_LIMIT               900.0f
#define GRAY_PID_D_FILTER_KEEP                0.65f

#define GRAY_PID_BASE_PWM                   1800
#define GRAY_PID_MAX_PWM                    2500
#define GRAY_PID_CURVE_SLOWDOWN_PER_UNIT      60.0f
#define GRAY_PID_SEARCH_BASE_PWM             950
#define GRAY_PID_SEARCH_CORRECTION_PWM       950

/*
 * The 30% tracking command can keep an MC520P30 turning, but may not
 * overcome gearbox and tyre static friction.  Apply this short kick only
 * when either wheel changes from stopped to commanded-on.
 */
#define GRAY_PID_START_KICK_PWM              2400
#define GRAY_PID_START_KICK_MS                200U

#define GRAY_PID_CONTROL_PERIOD_MS            10U
#define GRAY_PID_DISPLAY_PERIOD_CYCLES        10U
#define GRAY_PID_TELEMETRY_PERIOD_CYCLES       5U
#define GRAY_PID_LOST_SEARCH_CYCLES           30U
#define GRAY_PID_START_DELAY_SECONDS           3U

#if GRAY_PID_MAX_PWM > MOTOR_PWM_COMMAND_MAX
#error "GRAY_PID_MAX_PWM exceeds the configured motor PWM period"
#endif

#if GRAY_PID_START_KICK_PWM > MOTOR_PWM_COMMAND_MAX
#error "GRAY_PID_START_KICK_PWM exceeds the configured motor PWM period"
#endif

#if (GRAY_PID_START_KICK_MS % GRAY_PID_CONTROL_PERIOD_MS) != 0U
#error "GRAY_PID_START_KICK_MS must be a multiple of the control period"
#endif

void gray_pid_tracking_test_run(void);

#endif
