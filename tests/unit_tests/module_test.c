#include "tests.h"

#include <stdio.h>

#include "beep.h"
#include "delay.h"
#include "encoder_user.h"
#include "gimbal_uart.h"
#include "gray_detection.h"
#include "gray_pid_tracking_test.h"
#include "gyro_pid_control_test.h"
#include "h24_figure_eight.h"
#include "hal_uart.h"
#include "lengke_gyro.h"
#include "maix_cam.h"
#include "motor_speed_pid_test.h"
#include "motor_user.h"
#include "oled_driver.h"
#include "board_led.h"
#include "systick.h"
#include "ui_button.h"

static void test_display(const char *title, const char *line1, const char *line2,
    const char *line3)
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    u8g2_DrawStr(&u8g2, 0U, 12U, title);
    u8g2_DrawStr(&u8g2, 0U, 28U, line1);
    u8g2_DrawStr(&u8g2, 0U, 42U, line2);
    u8g2_DrawStr(&u8g2, 0U, 56U, line3);
    u8g2_SendBuffer(&u8g2);
}

static void test_display_init(const char *title)
{
    u8g2_Init();
    test_display(title, "Starting...", "", "");
}

static void test_format_one_decimal(char *output, size_t output_size,
    const char *label, float value)
{
    int32_t scaled_value = (int32_t) (value * 10.0f);
    uint32_t magnitude = scaled_value < 0 ?
        (uint32_t) (-scaled_value) : (uint32_t) scaled_value;

    (void) snprintf(output, output_size, "%s%s%lu.%lu", label,
        scaled_value < 0 ? "-" : "", (unsigned long) (magnitude / 10U),
        (unsigned long) (magnitude % 10U));
}

static void test_oled(void)
{
    uint8_t bar_x = 2U;

    /*
     * Test steps:
     * 1. Use the production u8g2_Init() path and PA16/PA17 software I2C.
     * 2. Draw a border, text, and a moving bar into the U8g2 frame buffer.
     * 3. Send the complete frame repeatedly through u8g2_SendBuffer().
     * Pass condition: the OLED shows the test pattern and the bar moves.
     */
    u8g2_Init();
    board_led_set(true);

    for (;;) {
        u8g2_ClearBuffer(&u8g2);
        u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
        u8g2_DrawFrame(&u8g2, 0U, 0U, 128U, 64U);
        u8g2_DrawStr(&u8g2, 16U, 18U, "OLED SW I2C");
        u8g2_DrawStr(&u8g2, 18U, 34U, "U8G2 TEST");
        u8g2_DrawFrame(&u8g2, 8U, 46U, 112U, 10U);
        u8g2_DrawBox(&u8g2, bar_x, 48U, 16U, 6U);
        u8g2_SendBuffer(&u8g2);

        bar_x = (uint8_t) (bar_x + 8U);
        if (bar_x > 102U) {
            bar_x = 10U;
        }
        delay_ms(200U);
    }
}

static void test_board_led(void)
{
    /*
     * Test steps:
     * 1. Drive the Tianmengxing onboard B22 LED from PB22.
     * 2. Toggle it once per second.
     * Pass condition: the onboard B22 LED turns on and off repeatedly.
     */
    test_display_init("BOARD LED TEST");

    for (;;) {
        board_led_set(true);
        test_display("BOARD LED TEST", "B22 LED ON", "PB22 = HIGH", "active high");
        delay_ms(1000U);
        board_led_set(false);
        test_display("BOARD LED TEST", "B22 LED OFF", "PB22 = LOW", "repeats");
        delay_ms(1000U);
    }
}

static void test_buzzer(void)
{
    test_display_init("BUZZER TEST");
    beep_init();

    for (;;) {
        beep_on();
        test_display("BUZZER TEST", "PB3 EN = HIGH", "BUZZER ON", "active high");
        delay_ms(500U);
        beep_off();
        test_display("BUZZER TEST", "PB3 EN = LOW", "BUZZER OFF", "");
        delay_ms(1000U);
    }
}

static void test_buttons(void)
{
    char first_row[22];
    char second_row[22];

    test_display_init("4 BUTTON TEST");

    for (;;) {
        bool key1 = user_button_is_pressed(BUTTON_RIGHT);
        bool key2 = user_button_is_pressed(BUTTON_LEFT);
        bool key3 = user_button_is_pressed(BUTTON_DOWN);
        bool key4 = user_button_is_pressed(BUTTON_UP);

        (void) snprintf(first_row, sizeof(first_row), "K1:%c K2:%c",
            key1 ? 'P' : '-', key2 ? 'P' : '-');
        (void) snprintf(second_row, sizeof(second_row), "K3:%c K4:%c",
            key3 ? 'P' : '-', key4 ? 'P' : '-');
        board_led_set(key1 || key2 || key3 || key4);
        test_display("4 BUTTON TEST", first_row, second_row,
            "26 27 B25 B20");
        delay_ms(20U);
    }
}

static void test_motor(void)
{
    int both_forward_pwms[2] = {
        MOTOR_DIRECTION_TEST_PWM, MOTOR_DIRECTION_TEST_PWM};
    int both_backward_pwms[2] = {
        -MOTOR_DIRECTION_TEST_PWM, -MOTOR_DIRECTION_TEST_PWM};
    int stop_pwms[2] = {0, 0};

    char left_rpm_text[22];
    char right_rpm_text[22];

    /* Two seconds forward, then two seconds reverse, with live wheel RPM. */
    systick_init();
    test_display_init("MOTOR RPM TEST");
    encoder_application_init();
    motor_init();

    for (;;) {
        uint32_t phase_start_ms;
        uint32_t previous_sample_ms;
        int32_t previous_left_count;
        int32_t previous_right_count;

        board_led_set(true);
        motor_set_pwms(both_forward_pwms);
        phase_start_ms = get_ms();
        previous_sample_ms = phase_start_ms;
        previous_left_count = encoder_manager_read(&robot_encoder_manager, 0U);
        previous_right_count = encoder_manager_read(&robot_encoder_manager, 1U);

        while ((uint32_t) (get_ms() - phase_start_ms) < 2000U) {
            uint32_t now_ms;
            uint32_t sample_ms;
            int32_t left_count;
            int32_t right_count;
            float left_rpm;
            float right_rpm;

            delay_ms(100U);
            now_ms = get_ms();
            sample_ms = now_ms - previous_sample_ms;
            left_count = encoder_manager_read(&robot_encoder_manager, 0U);
            right_count = encoder_manager_read(&robot_encoder_manager, 1U);
            left_rpm = ((float) (left_count - previous_left_count) * 60000.0f) /
                ((float) MOTOR_ENCODER_COUNTS_PER_OUTPUT_REV * (float) sample_ms);
            right_rpm = ((float) (right_count - previous_right_count) * 60000.0f) /
                ((float) MOTOR_ENCODER_COUNTS_PER_OUTPUT_REV * (float) sample_ms);
            if (MOTOR_LEFT_ENCODER_INVERTED) {
                left_rpm = -left_rpm;
            }
            if (MOTOR_RIGHT_ENCODER_INVERTED) {
                right_rpm = -right_rpm;
            }
            test_format_one_decimal(left_rpm_text, sizeof(left_rpm_text),
                "L RPM: ", left_rpm);
            test_format_one_decimal(right_rpm_text, sizeof(right_rpm_text),
                "R RPM: ", right_rpm);
            test_display("MOTOR RPM TEST", "FORWARD  2s",
                left_rpm_text, right_rpm_text);
            previous_sample_ms = now_ms;
            previous_left_count = left_count;
            previous_right_count = right_count;
        }

        motor_set_pwms(stop_pwms);
        board_led_set(false);
        delay_ms(MOTOR_DIRECTION_CHANGE_COAST_MS);

        board_led_set(true);
        motor_set_pwms(both_backward_pwms);
        phase_start_ms = get_ms();
        previous_sample_ms = phase_start_ms;
        previous_left_count = encoder_manager_read(&robot_encoder_manager, 0U);
        previous_right_count = encoder_manager_read(&robot_encoder_manager, 1U);

        while ((uint32_t) (get_ms() - phase_start_ms) < 2000U) {
            uint32_t now_ms;
            uint32_t sample_ms;
            int32_t left_count;
            int32_t right_count;
            float left_rpm;
            float right_rpm;

            delay_ms(100U);
            now_ms = get_ms();
            sample_ms = now_ms - previous_sample_ms;
            left_count = encoder_manager_read(&robot_encoder_manager, 0U);
            right_count = encoder_manager_read(&robot_encoder_manager, 1U);
            left_rpm = ((float) (left_count - previous_left_count) * 60000.0f) /
                ((float) MOTOR_ENCODER_COUNTS_PER_OUTPUT_REV * (float) sample_ms);
            right_rpm = ((float) (right_count - previous_right_count) * 60000.0f) /
                ((float) MOTOR_ENCODER_COUNTS_PER_OUTPUT_REV * (float) sample_ms);
            if (MOTOR_LEFT_ENCODER_INVERTED) {
                left_rpm = -left_rpm;
            }
            if (MOTOR_RIGHT_ENCODER_INVERTED) {
                right_rpm = -right_rpm;
            }
            test_format_one_decimal(left_rpm_text, sizeof(left_rpm_text),
                "L RPM: ", left_rpm);
            test_format_one_decimal(right_rpm_text, sizeof(right_rpm_text),
                "R RPM: ", right_rpm);
            test_display("MOTOR RPM TEST", "REVERSE  2s",
                left_rpm_text, right_rpm_text);
            previous_sample_ms = now_ms;
            previous_left_count = left_count;
            previous_right_count = right_count;
        }

        motor_set_pwms(stop_pwms);
        board_led_set(false);
        delay_ms(MOTOR_DIRECTION_CHANGE_COAST_MS);
    }
}

static void test_encoder(void)
{
    int forward_pwms[2] = {
        MOTOR_ENCODER_TEST_PWM, MOTOR_ENCODER_TEST_PWM};
    int stop_pwms[2] = {0, 0};
    char motor1_text[22];
    char motor2_text[22];

    /*
     * Test steps:
     * 1. Start both motors at a low speed for two seconds.
     * 2. Stop both motors and display the two quadrature counts.
     * 3. Repeat; each count must change while the corresponding wheel turns.
     * Pass condition: motor C PB13/PB12 and motor D PA29/PB26 counts are non-zero.
     */
    test_display_init("ENCODER TEST");
    encoder_application_init();
    motor_init();

    for (;;) {
        board_led_set(true);
        test_display("ENCODER TEST", "RUNNING 2s", "C PB13/PB12", "D PA29/PB26");
        motor_set_pwms(forward_pwms);
        delay_ms(2000U);
        motor_set_pwms(stop_pwms);

        (void) snprintf(motor1_text, sizeof(motor1_text), "L count: %ld",
            (long) encoder_manager_read(&robot_encoder_manager, 0U));
        (void) snprintf(motor2_text, sizeof(motor2_text), "R count: %ld",
            (long) encoder_manager_read(&robot_encoder_manager, 1U));
        board_led_set(false);
        test_display("ENCODER TEST", motor1_text, motor2_text, "turn wheels to verify");
        delay_ms(2500U);
    }
}

static void test_gray_sensor(void)
{
    char raw_text[22];
    char bit_text[22];

    /*
     * Test steps:
     * 1. Clock eight bits from the auxiliary board using PB27 (CLK).
     * 2. Read the data stream from PB23 (DAT) and show it in hex and binary.
     * 3. Move black and white material under each sensor channel.
     * Pass condition: the displayed bit pattern changes with the covered channel.
     */
    test_display_init("GRAY SENSOR TEST");
    gray_detection_init();

    for (;;) {
        uint8_t raw_value = (uint8_t) gray_read_byte();
        uint8_t display_value = (uint8_t) ((raw_value << 1U) | (raw_value >> 7U));

        /* The auxiliary board reports channels as bit6..bit0,bit7. */
        (void) snprintf(raw_text, sizeof(raw_text), "DATA: 0x%02X", display_value);
        (void) snprintf(bit_text, sizeof(bit_text), "BIT: %c%c%c%c%c%c%c%c",
            (display_value & 0x80U) ? '1' : '0', (display_value & 0x40U) ? '1' : '0',
            (display_value & 0x20U) ? '1' : '0', (display_value & 0x10U) ? '1' : '0',
            (display_value & 0x08U) ? '1' : '0', (display_value & 0x04U) ? '1' : '0',
            (display_value & 0x02U) ? '1' : '0', (display_value & 0x01U) ? '1' : '0');
        board_led_set(raw_value != 0U);
        test_display("GRAY SENSOR", raw_text, bit_text, "CLK PB27 DAT PB23");
        delay_ms(100U);
    }
}

static void test_gyro(void)
{
    static const uint32_t baud_rates[] = {
        115200U, 9600U, 4800U, 19200U,
        38400U, 57600U, 230400U, 2400U
    };
    char yaw_text[22];
    char rate_text[22];
    char frame_text[22];
    char baud_text[22];
    float yaw;
    float z_rate;
    bool baud_found = false;

    /*
     * Test steps:
     * 1. Enable UART1 RX interrupt on PA9; the gyro TX must connect to PA9.
     * 2. Parse Lengke 5-byte frames: 0x5A/0xAA is Z rate and 0x5A/0xBB is Yaw.
     * 3. Rotate the sensor around its Z axis and observe Yaw and Z rate.
     * Pass condition: frame count becomes non-zero and both values change.
     */
    test_display_init("LENGKE GYRO");
    lengke_gyro_init();

    /* The manual specifies a maximum 500 ms startup time. */
    test_display("LENGKE GYRO", "Wait module 500ms", "UART1 RX=PA9", "TX=PA8 MFCLK");
    delay_ms(600U);

    /*
     * The manual contradicts itself: its summary says 115200 is the default,
     * while the BAUD register page says default 0x0002 (9600). Search every
     * documented rate and accept only checksum-valid 5-byte gyro frames.
     */
    for (size_t index = 0U; index < sizeof(baud_rates) / sizeof(baud_rates[0]); index++) {
        lengke_gyro_set_host_baud_rate(baud_rates[index]);
        (void) snprintf(baud_text, sizeof(baud_text), "Try baud: %lu",
            (unsigned long) baud_rates[index]);
        test_display("LENGKE GYRO", baud_text, "Module TX -> PA9", "Scanning frames...");
        delay_ms(800U);

        if (lengke_gyro_has_valid_data()) {
            baud_found = true;
            break;
        }
    }

    if (baud_found) {
        (void) snprintf(baud_text, sizeof(baud_text), "Found: %lu",
            (unsigned long) lengke_gyro_get_host_baud_rate());
        test_display("LENGKE GYRO", baud_text, "Yaw zero...", "Keep gyro still");
        sendCaliYawCommand();
        delay_ms(300U);
    } else {
        /* Return to the value stated in the manual's parameter summary. */
        lengke_gyro_set_host_baud_rate(115200U);
    }

    for (;;) {
        yaw = Yaw();
        z_rate = GyroZ();
        test_format_one_decimal(yaw_text, sizeof(yaw_text), "Yaw  : ", yaw);
        test_format_one_decimal(rate_text, sizeof(rate_text), "Z rate: ", z_rate);
        (void) snprintf(frame_text, sizeof(frame_text), "B:%lu R:%lu O:%lu",
            (unsigned long) lengke_gyro_get_host_baud_rate(),
            (unsigned long) lengke_gyro_get_rx_byte_count(),
            (unsigned long) lengke_gyro_get_valid_frame_count());
        board_led_set(lengke_gyro_has_valid_data());
        test_display("LENGKE GYRO", yaw_text, rate_text, frame_text);
        delay_ms(100U);
    }
}

static void test_camera_uart(void)
{
    char count_text[22];
    char last_text[22];

    /*
     * Test steps:
     * 1. Enable CAM_UART RX interrupt on PB18 at 115200 baud.
     * 2. Start MaixCam Pro or make it send any serial output/data.
     * 3. Observe the received byte counter and the most recent byte value.
     * Pass condition: RX count increases when MaixCam sends data to PB18.
     */
    test_display_init("MAIXCAM UART");
    (void) camera_init();

    for (;;) {
        camera_process();
        (void) snprintf(count_text, sizeof(count_text), "RX count: %lu",
            (unsigned long) camera_get_rx_byte_count());
        (void) snprintf(last_text, sizeof(last_text), "Last: 0x%02X",
            camera_get_last_rx_byte());
        board_led_set(camera_get_rx_byte_count() != 0U);
        test_display("MAIXCAM UART", count_text, last_text, "RX PB18 TX PA21");
        delay_ms(100U);
    }
}

static void test_debug_uart(void)
{
    char rx_text[22];
    uint32_t rx_count = 0U;
    uint8_t last_byte = 0U;

    test_display_init("DEBUG UART3");

    for (;;) {
        usart_printf(DEBUG_UART_INST,
            "DEBUG UART3 TX=PA14 RX=PA25 heartbeat %lu\r\n",
            (unsigned long) rx_count);

        for (uint32_t poll = 0U; poll < 10U; poll++) {
            while (!DL_UART_Main_isRXFIFOEmpty(DEBUG_UART_INST)) {
                last_byte = DL_UART_Main_receiveData(DEBUG_UART_INST);
                rx_count++;
                usart_send_bytes(DEBUG_UART_INST, &last_byte, 1U);
            }
            delay_ms(50U);
        }

        (void) snprintf(rx_text, sizeof(rx_text), "RX:%lu last:%02X",
            (unsigned long) rx_count, last_byte);
        board_led_set(rx_count != 0U);
        test_display("DEBUG UART3", "TX PA14 RX PA25", rx_text,
            "115200 8N1");
    }
}

static void test_gimbal_uart(void)
{
    char count_text[22];
    uint32_t pass_count = 0U;
    uint32_t error_count = 0U;
    uint8_t tx_byte = 0U;
    uint8_t rx_byte;

    test_display_init("GIMBAL UART0");
    gimbal_uart_init();

    for (;;) {
        gimbal_uart_send_byte(tx_byte);
        delay_ms(5U);

        if (gimbal_uart_receive_byte(&rx_byte)) {
            if (rx_byte == tx_byte) {
                pass_count++;
            } else {
                error_count++;
            }
        }

        (void) snprintf(count_text, sizeof(count_text), "OK:%lu ERR:%lu",
            (unsigned long) pass_count, (unsigned long) error_count);
        board_led_set(pass_count != 0U && error_count == 0U);
        test_display("GIMBAL UART0", "LOOP PA0 -> PA1", count_text,
            "disconnect gimbal");
        tx_byte++;
        delay_ms(245U);
    }
}

static void test_oled_i2c(void)
{
    char sda_text[22];
    char scl_text[22];
    bool sda_high;
    bool scl_high;

    /*
     * Test steps:
     * 1. Read the idle levels of OLED SDA=PA16 and SCL=PA17.
     * 2. Verify the external 3.3 V pull-ups are fitted and both lines idle high.
     * 3. This checks the software-I2C wiring used by the OLED driver.
     * Pass condition: both SDA and SCL display HIGH when no transfer is active.
     */
    test_display_init("OLED I2C");

    for (;;) {
        sda_high = DL_GPIO_readPins(PORTA_PORT, PORTA_OLED_SDA_PIN) != 0U;
        scl_high = DL_GPIO_readPins(PORTA_PORT, PORTA_OLED_SCL_PIN) != 0U;
        (void) snprintf(sda_text, sizeof(sda_text), "SDA PA16: %s",
            sda_high ? "HIGH" : "LOW");
        (void) snprintf(scl_text, sizeof(scl_text), "SCL PA17: %s",
            scl_high ? "HIGH" : "LOW");
        board_led_set(sda_high && scl_high);
        test_display("OLED I2C", sda_text, scl_text, "need 3V3 pullups");
        delay_ms(250U);
    }
}

void module_test_run(void)
{
#if MODULE_TEST_SELECTED == MODULE_TEST_OLED
    test_oled();
#elif MODULE_TEST_SELECTED == MODULE_TEST_BOARD_LED
    test_board_led();
#elif MODULE_TEST_SELECTED == MODULE_TEST_BUZZER
    test_buzzer();
#elif MODULE_TEST_SELECTED == MODULE_TEST_MOTOR
    test_motor();
#elif MODULE_TEST_SELECTED == MODULE_TEST_ENCODER
    test_encoder();
#elif MODULE_TEST_SELECTED == MODULE_TEST_BUTTONS
    test_buttons();
#elif MODULE_TEST_SELECTED == MODULE_TEST_GRAY_SENSOR
    test_gray_sensor();
#elif MODULE_TEST_SELECTED == MODULE_TEST_GYRO
    test_gyro();
#elif MODULE_TEST_SELECTED == MODULE_TEST_CAMERA_UART
    test_camera_uart();
#elif MODULE_TEST_SELECTED == MODULE_TEST_OLED_I2C
    test_oled_i2c();
#elif MODULE_TEST_SELECTED == MODULE_TEST_GRAY_PID_TRACKING
    gray_pid_tracking_test_run();
#elif MODULE_TEST_SELECTED == MODULE_TEST_GYRO_PID_CONTROL
    gyro_pid_control_test_run();
#elif MODULE_TEST_SELECTED == MODULE_TEST_MOTOR_SPEED_PID
    motor_speed_pid_test_run();
#elif MODULE_TEST_SELECTED == MODULE_TEST_DEBUG_UART
    test_debug_uart();
#elif MODULE_TEST_SELECTED == MODULE_TEST_GIMBAL_UART
    test_gimbal_uart();
#elif MODULE_TEST_SELECTED == MODULE_TEST_H24_FIGURE_EIGHT
    h24_figure_eight_run();
#else
#error "MODULE_TEST_SELECTED is not a valid module test ID"
#endif
}
