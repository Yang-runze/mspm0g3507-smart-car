#ifndef GIMBAL_UART_H
#define GIMBAL_UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Raw UART transport for the gimbal controller: UART0 TX=PA0, RX=PA1. */
void gimbal_uart_init(void);
void gimbal_uart_send_byte(uint8_t data);
void gimbal_uart_send_bytes(const uint8_t *data, size_t length);
bool gimbal_uart_receive_byte(uint8_t *data);

#endif
