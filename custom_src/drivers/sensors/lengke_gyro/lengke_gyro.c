#include "lengke_gyro.h"

#include "delay.h"

#define LENGKE_GYRO_UART GYRO_UART_INST

#define LENGKE_FRAME_HEADER 0x5AU
#define LENGKE_FRAME_SIZE 5U
#define LENGKE_FRAME_Z_RATE 0xAAU
#define LENGKE_FRAME_YAW 0xBBU

static unsigned char receive_buffer[11];
static unsigned char receive_count;

volatile LengkeGyroData g_lengke_gyro;

static void lengke_gyro_send_bytes(const uint8_t *data, uint32_t length)
{
    for (uint32_t index = 0U; index < length; index++) {
        while (DL_UART_isBusy(LENGKE_GYRO_UART)) {
        }
        DL_UART_Main_transmitData(LENGKE_GYRO_UART, data[index]);
    }
}

static void lengke_gyro_clear_receive_state(void)
{
    receive_count = 0U;
    g_lengke_gyro.yaw_deg = 0.0f;
    g_lengke_gyro.z_rate_dps = 0.0f;
    g_lengke_gyro.rx_byte_count = 0U;
    g_lengke_gyro.valid_frame_count = 0U;
    g_lengke_gyro.checksum_error_count = 0U;
    g_lengke_gyro.latest_frame_type = 0U;
}

/*
 * This is the manufacturer's CopeSerial2Data() parser, adapted only to store
 * the result in this project's data structure. The wire format is unchanged.
 */
void CopeSerial2Data(unsigned char data)
{
    unsigned char sum;
    int16_t raw_value;

    g_lengke_gyro.rx_byte_count++;
    receive_buffer[receive_count++] = data;

    if (receive_buffer[0] != LENGKE_FRAME_HEADER) {
        receive_count = 0U;
        return;
    }

    if (receive_count < LENGKE_FRAME_SIZE) {
        return;
    }

    sum = (unsigned char) (receive_buffer[0] + receive_buffer[1] +
        receive_buffer[2] + receive_buffer[3]);

    if (sum != receive_buffer[4]) {
        g_lengke_gyro.checksum_error_count++;
        receive_count = 0U;
        return;
    }

    raw_value = (int16_t) (((uint16_t) receive_buffer[3] << 8U) |
        (uint16_t) receive_buffer[2]);

    if (receive_buffer[1] == LENGKE_FRAME_Z_RATE) {
        g_lengke_gyro.z_rate_dps = ((float) raw_value * 2000.0f) / 32768.0f;
    } else if (receive_buffer[1] == LENGKE_FRAME_YAW) {
        g_lengke_gyro.yaw_deg = ((float) raw_value * 180.0f) / 32768.0f;
    } else {
        receive_count = 0U;
        return;
    }

    g_lengke_gyro.latest_frame_type = receive_buffer[1];
    g_lengke_gyro.valid_frame_count++;
    receive_count = 0U;
}

void lengke_gyro_init(void)
{
    lengke_gyro_clear_receive_state();
    g_lengke_gyro.host_baud_rate = GYRO_UART_BAUD_RATE;

    NVIC_ClearPendingIRQ(GYRO_UART_INST_INT_IRQN);
    NVIC_EnableIRQ(GYRO_UART_INST_INT_IRQN);
}

void lengke_gyro_uart_irq_handler(void)
{
    /* UART1 RX interrupt: gyro TX is connected to PA9. */
    switch (DL_UART_Main_getPendingInterrupt(LENGKE_GYRO_UART)) {
        case DL_UART_MAIN_IIDX_RX:
            CopeSerial2Data(
                DL_UART_Main_receiveData(LENGKE_GYRO_UART));
            break;

        default:
            break;
    }
}

void lengke_gyro_reset_received_data(void)
{
    uint32_t baud_rate = g_lengke_gyro.host_baud_rate;

    lengke_gyro_clear_receive_state();
    g_lengke_gyro.host_baud_rate = baud_rate;
}

void lengke_gyro_set_host_baud_rate(uint32_t baud_rate)
{
    NVIC_DisableIRQ(GYRO_UART_INST_INT_IRQN);
    DL_UART_Main_disableInterrupt(
        LENGKE_GYRO_UART, DL_UART_MAIN_INTERRUPT_RX);

    while (!DL_UART_Main_isRXFIFOEmpty(LENGKE_GYRO_UART)) {
        (void) DL_UART_Main_receiveData(LENGKE_GYRO_UART);
    }

    DL_UART_Main_disable(LENGKE_GYRO_UART);
    DL_UART_Main_configBaudRate(
        LENGKE_GYRO_UART, GYRO_UART_INST_FREQUENCY, baud_rate);
    DL_UART_Main_enableInterrupt(
        LENGKE_GYRO_UART, DL_UART_MAIN_INTERRUPT_RX);
    DL_UART_Main_enable(LENGKE_GYRO_UART);

    lengke_gyro_clear_receive_state();
    g_lengke_gyro.host_baud_rate = baud_rate;
    NVIC_ClearPendingIRQ(GYRO_UART_INST_INT_IRQN);
    NVIC_EnableIRQ(GYRO_UART_INST_INT_IRQN);
}

uint32_t lengke_gyro_get_host_baud_rate(void)
{
    return g_lengke_gyro.host_baud_rate;
}

float lengke_gyro_get_yaw(void)
{
    return g_lengke_gyro.yaw_deg;
}

float lengke_gyro_get_z_rate(void)
{
    return g_lengke_gyro.z_rate_dps;
}

float GyroZ(void)
{
    return g_lengke_gyro.z_rate_dps;
}

float Yaw(void)
{
    return g_lengke_gyro.yaw_deg;
}

bool lengke_gyro_has_valid_data(void)
{
    return g_lengke_gyro.valid_frame_count != 0U;
}

uint32_t lengke_gyro_get_rx_byte_count(void)
{
    return g_lengke_gyro.rx_byte_count;
}

uint32_t lengke_gyro_get_valid_frame_count(void)
{
    return g_lengke_gyro.valid_frame_count;
}

uint32_t lengke_gyro_get_checksum_error_count(void)
{
    return g_lengke_gyro.checksum_error_count;
}

void lengke_gyro_set_yaw_zero(void)
{
    static const uint8_t unlock_command[] = {0x55U, 0xAAU, 0x13U, 0x8EU, 0x5FU};
    static const uint8_t yaw_zero_command[] = {0x55U, 0xAAU, 0x15U, 0x00U, 0x00U};
    static const uint8_t save_command[] = {0x55U, 0xAAU, 0x00U, 0x00U, 0x00U};

    lengke_gyro_send_bytes(unlock_command, sizeof(unlock_command));
    delay_ms(100U);
    lengke_gyro_send_bytes(yaw_zero_command, sizeof(yaw_zero_command));
    delay_ms(100U);
    lengke_gyro_send_bytes(save_command, sizeof(save_command));
}

void sendCaliYawCommand(void)
{
    lengke_gyro_set_yaw_zero();
}

void lengke_gyro_calibrate_bias(void)
{
    static const uint8_t unlock_command[] = {0x55U, 0xAAU, 0x13U, 0x8EU, 0x5FU};
    static const uint8_t bias_command[] = {0x55U, 0xAAU, 0x0AU, 0x01U, 0x00U};
    static const uint8_t save_command[] = {0x55U, 0xAAU, 0x00U, 0x00U, 0x00U};

    lengke_gyro_send_bytes(unlock_command, sizeof(unlock_command));
    delay_ms(100U);
    lengke_gyro_send_bytes(bias_command, sizeof(bias_command));
    delay_ms(21000U);
    lengke_gyro_send_bytes(save_command, sizeof(save_command));
}

void performCaliBias(void)
{
    lengke_gyro_calibrate_bias();
}
