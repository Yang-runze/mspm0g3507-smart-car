#ifndef SERVO_USER_H
#define SERVO_USER_H

#include <stdint.h>

#define SERVO_CHANNEL_0 0U
#define SERVO_CHANNEL_1 1U

void servo_init(void);
void servo_set_pulse_us(uint8_t channel, uint16_t pulse_us);

#endif
