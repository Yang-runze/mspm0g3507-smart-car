#ifndef H24_FIGURE_EIGHT_H
#define H24_FIGURE_EIGHT_H

#include "gray_pid_tracking_test.h"
#include "gyro_pid_control_test.h"
#include "motor_config.h"

/*
 * 2024 H problem, figure-eight path only:
 *
 *   A -> C : gyro heading hold on the unmarked diagonal
 *   C -> B : gray PID on the right 40 cm-radius semicircle
 *   B -> D : gyro heading hold on the unmarked diagonal
 *   D -> A : gray PID on the left 40 cm-radius semicircle
 *
 * Start with the sensor row centered on point A and the car facing A -> B.
 * K1 selects 1...4 laps; K2 locks the selection and starts a 2 s countdown.
 */

#define H24_FIGURE_EIGHT_CONTROL_PERIOD_MS             10U
#define H24_FIGURE_EIGHT_DISPLAY_PERIOD_CYCLES         10U
#define H24_FIGURE_EIGHT_START_DELAY_SECONDS            2U

/* Standard 2024 H field geometry: horizontal 100 cm, vertical 80 cm. */
#define H24_FIRST_DIAGONAL_YAW_DEG                   38.66f
#define H24_SECOND_DIAGONAL_YAW_DEG                 141.34f

/* Encoder geometry: XTARK 65 mm blue wheel and calibrated 1560 counts/rev. */
#define H24_WHEEL_DIAMETER_MM                         65.0f
#define H24_ENCODER_COUNTS_PER_REV \
    ((float) MOTOR_ENCODER_COUNTS_PER_OUTPUT_REV)

/*
 * Point detection is sensor-led and distance-gated. The nominal diagonal is
 * sqrt(1000^2 + 800^2) = 1280.6 mm and each semicircle is pi*400 = 1256.6 mm.
 */
#define H24_DIAGONAL_LINE_ENABLE_DISTANCE_MM          950.0f
#define H24_DIAGONAL_MAX_DISTANCE_MM                 1550.0f
#define H24_ARC_END_ENABLE_DISTANCE_MM               1000.0f
#define H24_ARC_MAX_DISTANCE_MM                      1550.0f
#define H24_ARC_END_ENABLE_YAW_DEG                    140.0f

/* Slow A -> B tangent exit, forward-only heading acquisition, normal travel. */
#define H24_START_EXIT_PWM                              800
#define H24_HEADING_ACQUIRE_BASE_PWM                    600
#define H24_DRIVE_MAX_PWM                              1800
#define H24_START_EXIT_MAX_DISTANCE_MM                 250.0f

#define H24_HEADING_TOLERANCE_DEG                        3.0f
#define H24_HEADING_RATE_TOLERANCE_DPS                  12.0f
#define H24_HEADING_SETTLE_CYCLES                        5U
#define H24_HEADING_ACQUIRE_TIMEOUT_CYCLES             250U

#define H24_LINE_CONFIRM_CYCLES                          3U
#define H24_ARC_LINE_RECOVERY_CYCLES                    50U
#define H24_POINT_LED_FLASH_MS                         100U
#define H24_GYRO_DATA_TIMEOUT_CYCLES                    30U

/* Confirm the tested gray and gyro loops still run at this module's period. */
#if H24_FIGURE_EIGHT_CONTROL_PERIOD_MS != GRAY_PID_CONTROL_PERIOD_MS
#error "H24 and gray PID control periods must match"
#endif

#if H24_FIGURE_EIGHT_CONTROL_PERIOD_MS != GYRO_PID_CONTROL_PERIOD_MS
#error "H24 and gyro PID control periods must match"
#endif

#if H24_DRIVE_MAX_PWM > MOTOR_PWM_COMMAND_MAX
#error "H24_DRIVE_MAX_PWM exceeds the configured motor PWM period"
#endif

#if (H24_POINT_LED_FLASH_MS % H24_FIGURE_EIGHT_CONTROL_PERIOD_MS) != 0U
#error "H24_POINT_LED_FLASH_MS must be a multiple of the control period"
#endif

void h24_figure_eight_run(void);

#endif
