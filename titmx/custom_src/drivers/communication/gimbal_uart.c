#include "gimbal_uart.h"

#include "hal_uart.h"
#include "ti_msp_dl_config.h"

void gimbal_uart_init(void)
{
    while (!DL_UART_Main_isRXFIFOEmpty(GIMBAL_UART_INST)) {
        (void) DL_UART_Main_receiveData(GIMBAL_UART_INST);
    }
}

void gimbal_uart_send_byte(uint8_t data)
{
    usart_send_bytes(GIMBAL_UART_INST, &data, 1U);
}

void gimbal_uart_send_bytes(const uint8_t *data, size_t length)
{
    if (data != NULL && length != 0U) {
        usart_send_bytes(GIMBAL_UART_INST, data, length);
    }
}

bool gimbal_uart_receive_byte(uint8_t *data)
{
    if (data == NULL || DL_UART_Main_isRXFIFOEmpty(GIMBAL_UART_INST)) {
        return false;
    }

    *data = DL_UART_Main_receiveData(GIMBAL_UART_INST);
    return true;
}
