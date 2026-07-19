#ifndef TESTS_H
#define TESTS_H

/*
 * Set MODULE_TEST_SELECTED to one item below, rebuild, and then flash.
 * Only the selected test is started.
 */
#define MODULE_TEST_OLED 1U
#define MODULE_TEST_RGB_LED 2U
#define MODULE_TEST_MOTOR 4U
#define MODULE_TEST_ENCODER 5U
#define MODULE_TEST_GRAY_SENSOR 7U
#define MODULE_TEST_GYRO 8U
#define MODULE_TEST_CAMERA_UART 9U
#define MODULE_TEST_CAMERA_I2C 10U
#define MODULE_TEST_GRAY_PID_TRACKING 11U
#define MODULE_TEST_GYRO_PID_CONTROL 12U
#define MODULE_TEST_MOTOR_SPEED_PID 13U

#ifndef MODULE_TEST_SELECTED
#define MODULE_TEST_SELECTED MODULE_TEST_GRAY_PID_TRACKING
#endif

void module_test_run(void);

#endif
