#ifndef ENCODER_APP_H__
#define ENCODER_APP_H__

#include "encoder.h"

typedef struct {
    GPIO_Regs *port;
    uint32_t pin_mask;
    uint16_t map_index;
} encoder_pin_config_t;

extern encoder_manager_t robot_encoder_manager;

void encoder_application_init(void);
void encoder_group1_irq_handler(void);

#endif
