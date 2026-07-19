/*
 * oled_driver.h
 *
 *  Created on: 2022年7月24日
 *      Author: Unicorn_Li
 */
#ifndef __OLED_DRIVER_H__
#define __OLED_DRIVER_H__

#include "stdint.h"
#include "stdbool.h"
#include "delay.h"
#include "u8g2.h"
#include "ti_msp_dl_config.h"


// 驱动模式选择
#define OLED_DRIVER_MODE_SPI 1
#define OLED_DRIVER_MODE_I2C 2

#define OLED_DRIVER_MODE OLED_DRIVER_MODE_I2C

#if OLED_DRIVER_MODE == OLED_DRIVER_MODE_SPI

#define OLED_RST_Clr() DL_GPIO_clearPins(PORTB_PORT, PORTB_OLED_RST_PIN)
#define OLED_RST_Set() DL_GPIO_setPins(PORTB_PORT, PORTB_OLED_RST_PIN)
#define OLED_DC_Clr()  DL_GPIO_clearPins(PORTB_PORT, PORTB_OLED_DC_PIN)
#define OLED_DC_Set()  DL_GPIO_setPins(PORTB_PORT, PORTB_OLED_DC_PIN)
#define OLED_CS_Clr()  DL_GPIO_clearPins(PORTB_PORT, PORTB_OLED_CS_PIN)
#define OLED_CS_Set()  DL_GPIO_setPins(PORTB_PORT, PORTB_OLED_CS_PIN)

#elif OLED_DRIVER_MODE == OLED_DRIVER_MODE_I2C

// 4-wire SSD1306 OLED: VCC, GND, SDA0=PA16, SCL0=PA17.
#define OLED_I2C_SDA_PORT   PORTA_PORT
#define OLED_I2C_SDA_PIN    PORTA_OLED_SDA_PIN

#define OLED_I2C_SCL_PORT   PORTA_PORT
#define OLED_I2C_SCL_PIN    PORTA_OLED_SCL_PIN

#define OLED_I2C_SCL_IOMUX PORTA_OLED_SCL_IOMUX
#define OLED_I2C_SDA_IOMUX PORTA_OLED_SDA_IOMUX

#define OLED_SDA_Clr() DL_GPIO_clearPins(OLED_I2C_SDA_PORT, OLED_I2C_SDA_PIN)
#define OLED_SDA_Set() DL_GPIO_setPins(OLED_I2C_SDA_PORT, OLED_I2C_SDA_PIN)

#define OLED_SCL_Clr() DL_GPIO_clearPins(OLED_I2C_SCL_PORT, OLED_I2C_SCL_PIN)
#define OLED_SCL_Set() DL_GPIO_setPins(OLED_I2C_SCL_PORT, OLED_I2C_SCL_PIN)

#endif // OLED_DRIVER_MODE_I2C

void u8g2_Init(void);
void oled_force_all_pixels(bool enable);

extern u8g2_t u8g2;

#endif // __OLED_DRIVER_H__
