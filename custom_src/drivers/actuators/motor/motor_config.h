#ifndef MOTOR_CONFIG_H
#define MOTOR_CONFIG_H

#include <stdbool.h>

/*
 * Robot drive motor: MC520P30_12V.
 *
 * The encoder produces 13 cycles per motor-shaft revolution on each channel.
 * The gearbox is 30:1 and the project decodes both edges of both A/B channels,
 * so one output-shaft revolution is 13 * 30 * 4 = 1560 counts.
 */
#define MOTOR_MODEL_RATED_VOLTAGE_MV                 12000U
#define MOTOR_MODEL_GEAR_RATIO                          30U
#define MOTOR_MODEL_NO_LOAD_RPM                        360U
#define MOTOR_MODEL_NO_LOAD_RPM_TOLERANCE               20U
#define MOTOR_MODEL_RATED_TORQUE_G_CM                  1500U
#define MOTOR_MODEL_ENCODER_CYCLES_PER_MOTOR_REV        13U
#define MOTOR_ENCODER_RECOMMENDED_SUPPLY_MV           3300U
#define MOTOR_ENCODER_QUADRATURE_MULTIPLIER               4U
#define MOTOR_ENCODER_CYCLES_PER_OUTPUT_REV \
    (MOTOR_MODEL_ENCODER_CYCLES_PER_MOTOR_REV * MOTOR_MODEL_GEAR_RATIO)
#define MOTOR_ENCODER_COUNTS_PER_OUTPUT_REV \
    (MOTOR_ENCODER_CYCLES_PER_OUTPUT_REV * MOTOR_ENCODER_QUADRATURE_MULTIPLIER)

/*
 * Motor_PWM1/TIMA0 and Motor_PWM2/TIMA1 are configured as 10 MHz
 * edge-aligned timers with period 3000, giving approximately 3.33 kHz PWM.
 * Each DRV8870 uses two PWM inputs; only one input is active at a time.
 */
#define MOTOR_PWM_TIMER_CLOCK_HZ                  10000000U
#define MOTOR_PWM_TIMER_PERIOD_COUNTS                 3000U
#define MOTOR_PWM_COMMAND_MAX                         3000
#define MOTOR_PWM_FREQUENCY_HZ \
    (MOTOR_PWM_TIMER_CLOCK_HZ / MOTOR_PWM_TIMER_PERIOD_COUNTS)

/* DRV8870 wiring: M1 IN1=PA15, IN2=PB20; M2 IN1=PB17, IN2=PA25. */

/* Conservative open-loop values used only by the basic motor tests. */
#define MOTOR_DIRECTION_TEST_PWM                      2100
#define MOTOR_ENCODER_TEST_PWM                         900
#define MOTOR_DIRECTION_CHANGE_COAST_MS                500U

/* Physical installation calibration. Change only after the lifted-wheel test. */
#define MOTOR_LEFT_DIRECTION_INVERTED                  true
#define MOTOR_RIGHT_DIRECTION_INVERTED                false
#define MOTOR_LEFT_ENCODER_INVERTED                   false
#define MOTOR_RIGHT_ENCODER_INVERTED                   true

#endif
