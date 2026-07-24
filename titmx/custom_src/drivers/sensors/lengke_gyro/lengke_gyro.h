#ifndef LENGKE_GYRO_H
#define LENGKE_GYRO_H

#include <stdbool.h>
#include <stdint.h>

#include "ti_msp_dl_config.h"

/*
 * Lengke Intelligent single-axis gyro UART protocol:
 *   5A AA dataL dataH checksum  -> Z-axis angular rate
 *   5A BB dataL dataH checksum  -> Yaw angle
 */
typedef struct {
    float yaw_deg;
    float z_rate_dps;
    uint32_t rx_byte_count;
    uint32_t valid_frame_count;
    uint32_t checksum_error_count;
    uint32_t host_baud_rate;
    uint8_t latest_frame_type;
} LengkeGyroData;

void lengke_gyro_init(void);
void lengke_gyro_uart_irq_handler(void);
void lengke_gyro_reset_received_data(void);
void lengke_gyro_set_host_baud_rate(uint32_t baud_rate);
uint32_t lengke_gyro_get_host_baud_rate(void);

float lengke_gyro_get_yaw(void);
float lengke_gyro_get_z_rate(void);
bool lengke_gyro_has_valid_data(void);
uint32_t lengke_gyro_get_rx_byte_count(void);
uint32_t lengke_gyro_get_valid_frame_count(void);
uint32_t lengke_gyro_get_checksum_error_count(void);

void lengke_gyro_set_yaw_zero(void);
void lengke_gyro_calibrate_bias(void);

/* Names retained from the manufacturer's example for easy comparison. */
void CopeSerial2Data(unsigned char data);
float GyroZ(void);
float Yaw(void);
void sendCaliYawCommand(void);
void performCaliBias(void);

extern volatile LengkeGyroData g_lengke_gyro;

#endif
