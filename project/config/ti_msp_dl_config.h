/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     80000000
/* Defines for SYSPLL_ERR_01 Workaround */
/* Represent 1.000 as 1000 */
#define FLOAT_TO_INT_SCALE                                               (1000U)
#define FCC_EXPECTED_RATIO                                                  2500
#define FCC_UPPER_BOUND                       (FCC_EXPECTED_RATIO * (1 + 0.003))
#define FCC_LOWER_BOUND                       (FCC_EXPECTED_RATIO * (1 - 0.003))

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);


/* Defines for Motor_PWM1 */
#define Motor_PWM1_INST                                                    TIMA0
#define Motor_PWM1_INST_IRQHandler                              TIMA0_IRQHandler
#define Motor_PWM1_INST_INT_IRQN                                (TIMA0_INT_IRQn)
#define Motor_PWM1_INST_CLK_FREQ                                        10000000
/* GPIO defines for channel 1 */
#define GPIO_Motor_PWM1_C1_PORT                                            GPIOB
#define GPIO_Motor_PWM1_C1_PIN                                    DL_GPIO_PIN_20
#define GPIO_Motor_PWM1_C1_IOMUX                                 (IOMUX_PINCM48)
#define GPIO_Motor_PWM1_C1_IOMUX_FUNC                IOMUX_PINCM48_PF_TIMA0_CCP1
#define GPIO_Motor_PWM1_C1_IDX                               DL_TIMER_CC_1_INDEX
/* GPIO defines for channel 2 */
#define GPIO_Motor_PWM1_C2_PORT                                            GPIOB
#define GPIO_Motor_PWM1_C2_PIN                                    DL_GPIO_PIN_17
#define GPIO_Motor_PWM1_C2_IOMUX                                 (IOMUX_PINCM43)
#define GPIO_Motor_PWM1_C2_IOMUX_FUNC                IOMUX_PINCM43_PF_TIMA0_CCP2
#define GPIO_Motor_PWM1_C2_IDX                               DL_TIMER_CC_2_INDEX
/* GPIO defines for channel 3 */
#define GPIO_Motor_PWM1_C3_PORT                                            GPIOA
#define GPIO_Motor_PWM1_C3_PIN                                    DL_GPIO_PIN_25
#define GPIO_Motor_PWM1_C3_IOMUX                                 (IOMUX_PINCM55)
#define GPIO_Motor_PWM1_C3_IOMUX_FUNC                IOMUX_PINCM55_PF_TIMA0_CCP3
#define GPIO_Motor_PWM1_C3_IDX                               DL_TIMER_CC_3_INDEX

/* Defines for Motor_PWM2 */
#define Motor_PWM2_INST                                                    TIMA1
#define Motor_PWM2_INST_IRQHandler                              TIMA1_IRQHandler
#define Motor_PWM2_INST_INT_IRQN                                (TIMA1_INT_IRQn)
#define Motor_PWM2_INST_CLK_FREQ                                        10000000
/* GPIO defines for channel 0 */
#define GPIO_Motor_PWM2_C0_PORT                                            GPIOA
#define GPIO_Motor_PWM2_C0_PIN                                    DL_GPIO_PIN_15
#define GPIO_Motor_PWM2_C0_IOMUX                                 (IOMUX_PINCM37)
#define GPIO_Motor_PWM2_C0_IOMUX_FUNC                IOMUX_PINCM37_PF_TIMA1_CCP0
#define GPIO_Motor_PWM2_C0_IDX                               DL_TIMER_CC_0_INDEX




/* Defines for I2C_0 */
#define I2C_0_INST                                                          I2C0
#define I2C_0_INST_IRQHandler                                    I2C0_IRQHandler
#define I2C_0_INST_INT_IRQN                                        I2C0_INT_IRQn
#define I2C_0_BUS_SPEED_HZ                                                100000
#define GPIO_I2C_0_SDA_PORT                                                GPIOA
#define GPIO_I2C_0_SDA_PIN                                        DL_GPIO_PIN_28
#define GPIO_I2C_0_IOMUX_SDA                                      (IOMUX_PINCM3)
#define GPIO_I2C_0_IOMUX_SDA_FUNC                       IOMUX_PINCM3_PF_I2C0_SDA
#define GPIO_I2C_0_SCL_PORT                                                GPIOA
#define GPIO_I2C_0_SCL_PIN                                        DL_GPIO_PIN_31
#define GPIO_I2C_0_IOMUX_SCL                                      (IOMUX_PINCM6)
#define GPIO_I2C_0_IOMUX_SCL_FUNC                       IOMUX_PINCM6_PF_I2C0_SCL


/* Defines for GYRO_UART */
#define GYRO_UART_INST                                                     UART3
#define GYRO_UART_INST_FREQUENCY                                         4000000
#define GYRO_UART_INST_IRQHandler                               UART3_IRQHandler
#define GYRO_UART_INST_INT_IRQN                                   UART3_INT_IRQn
#define GPIO_GYRO_UART_RX_PORT                                             GPIOB
#define GPIO_GYRO_UART_TX_PORT                                             GPIOB
#define GPIO_GYRO_UART_RX_PIN                                      DL_GPIO_PIN_3
#define GPIO_GYRO_UART_TX_PIN                                      DL_GPIO_PIN_2
#define GPIO_GYRO_UART_IOMUX_RX                                  (IOMUX_PINCM16)
#define GPIO_GYRO_UART_IOMUX_TX                                  (IOMUX_PINCM15)
#define GPIO_GYRO_UART_IOMUX_RX_FUNC                   IOMUX_PINCM16_PF_UART3_RX
#define GPIO_GYRO_UART_IOMUX_TX_FUNC                   IOMUX_PINCM15_PF_UART3_TX
#define GYRO_UART_BAUD_RATE                                             (115200)
#define GYRO_UART_IBRD_4_MHZ_115200_BAUD                                     (2)
#define GYRO_UART_FBRD_4_MHZ_115200_BAUD                                    (11)
/* Defines for CAM_UART */
#define CAM_UART_INST                                                      UART0
#define CAM_UART_INST_FREQUENCY                                         40000000
#define CAM_UART_INST_IRQHandler                                UART0_IRQHandler
#define CAM_UART_INST_INT_IRQN                                    UART0_INT_IRQn
#define GPIO_CAM_UART_RX_PORT                                              GPIOA
#define GPIO_CAM_UART_TX_PORT                                              GPIOA
#define GPIO_CAM_UART_RX_PIN                                      DL_GPIO_PIN_11
#define GPIO_CAM_UART_TX_PIN                                      DL_GPIO_PIN_10
#define GPIO_CAM_UART_IOMUX_RX                                   (IOMUX_PINCM22)
#define GPIO_CAM_UART_IOMUX_TX                                   (IOMUX_PINCM21)
#define GPIO_CAM_UART_IOMUX_RX_FUNC                    IOMUX_PINCM22_PF_UART0_RX
#define GPIO_CAM_UART_IOMUX_TX_FUNC                    IOMUX_PINCM21_PF_UART0_TX
#define CAM_UART_BAUD_RATE                                              (115200)
#define CAM_UART_IBRD_40_MHZ_115200_BAUD                                    (21)
#define CAM_UART_FBRD_40_MHZ_115200_BAUD                                    (45)
/* Defines for DEBUG_UART */
#define DEBUG_UART_INST                                                    UART2
#define DEBUG_UART_INST_FREQUENCY                                       40000000
#define DEBUG_UART_INST_IRQHandler                              UART2_IRQHandler
#define DEBUG_UART_INST_INT_IRQN                                  UART2_INT_IRQn
#define GPIO_DEBUG_UART_TX_PORT                                            GPIOA
#define GPIO_DEBUG_UART_TX_PIN                                    DL_GPIO_PIN_23
#define GPIO_DEBUG_UART_IOMUX_TX                                 (IOMUX_PINCM53)
#define GPIO_DEBUG_UART_IOMUX_TX_FUNC                  IOMUX_PINCM53_PF_UART2_TX
#define DEBUG_UART_BAUD_RATE                                            (115200)
#define DEBUG_UART_IBRD_40_MHZ_115200_BAUD                                  (21)
#define DEBUG_UART_FBRD_40_MHZ_115200_BAUD                                  (45)





/* Port definition for Pin Group PORTB */
#define PORTB_PORT                                                       (GPIOB)

/* Defines for LED_R: GPIOB.26 with pinCMx 57 on package pin 28 */
#define PORTB_LED_R_PIN                                         (DL_GPIO_PIN_26)
#define PORTB_LED_R_IOMUX                                        (IOMUX_PINCM57)
/* Defines for LED_G: GPIOB.27 with pinCMx 58 on package pin 29 */
#define PORTB_LED_G_PIN                                         (DL_GPIO_PIN_27)
#define PORTB_LED_G_IOMUX                                        (IOMUX_PINCM58)
/* Defines for LED_B: GPIOB.22 with pinCMx 50 on package pin 21 */
#define PORTB_LED_B_PIN                                         (DL_GPIO_PIN_22)
#define PORTB_LED_B_IOMUX                                        (IOMUX_PINCM50)
/* Defines for KEY1: GPIOB.12 with pinCMx 29 on package pin 64 */
#define PORTB_KEY1_PIN                                          (DL_GPIO_PIN_12)
#define PORTB_KEY1_IOMUX                                         (IOMUX_PINCM29)
/* Defines for KEY2: GPIOB.8 with pinCMx 25 on package pin 60 */
#define PORTB_KEY2_PIN                                           (DL_GPIO_PIN_8)
#define PORTB_KEY2_IOMUX                                         (IOMUX_PINCM25)
/* Defines for KEY3: GPIOB.9 with pinCMx 26 on package pin 61 */
#define PORTB_KEY3_PIN                                           (DL_GPIO_PIN_9)
#define PORTB_KEY3_IOMUX                                         (IOMUX_PINCM26)
/* Defines for KEY4: GPIOB.10 with pinCMx 27 on package pin 62 */
#define PORTB_KEY4_PIN                                          (DL_GPIO_PIN_10)
#define PORTB_KEY4_IOMUX                                         (IOMUX_PINCM27)
/* Defines for M1_ENCODER_B: GPIOB.24 with pinCMx 52 on package pin 23 */
// pins affected by this interrupt request:["M1_ENCODER_B"]
#define PORTB_INT_IRQN                                          (GPIOB_INT_IRQn)
#define PORTB_INT_IIDX                          (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define PORTB_M1_ENCODER_B_IIDX                             (DL_GPIO_IIDX_DIO24)
#define PORTB_M1_ENCODER_B_PIN                                  (DL_GPIO_PIN_24)
#define PORTB_M1_ENCODER_B_IOMUX                                 (IOMUX_PINCM52)
/* Defines for TRACK_CLK: GPIOB.15 with pinCMx 32 on package pin 3 */
#define PORTB_TRACK_CLK_PIN                                     (DL_GPIO_PIN_15)
#define PORTB_TRACK_CLK_IOMUX                                    (IOMUX_PINCM32)
/* Defines for TRACK_DAT: GPIOB.16 with pinCMx 33 on package pin 4 */
#define PORTB_TRACK_DAT_PIN                                     (DL_GPIO_PIN_16)
#define PORTB_TRACK_DAT_IOMUX                                    (IOMUX_PINCM33)
/* Port definition for Pin Group PORTA */
#define PORTA_PORT                                                       (GPIOA)

/* Defines for OLED_SDA: GPIOA.16 with pinCMx 38 on package pin 9 */
#define PORTA_OLED_SDA_PIN                                      (DL_GPIO_PIN_16)
#define PORTA_OLED_SDA_IOMUX                                     (IOMUX_PINCM38)
/* Defines for OLED_SCL: GPIOA.17 with pinCMx 39 on package pin 10 */
#define PORTA_OLED_SCL_PIN                                      (DL_GPIO_PIN_17)
#define PORTA_OLED_SCL_IOMUX                                     (IOMUX_PINCM39)
/* Defines for M1_ENCODER_A: GPIOA.22 with pinCMx 47 on package pin 18 */
// pins affected by this interrupt request:["M1_ENCODER_A","M2_ENCODER_A","M2_ENCODER_B"]
#define PORTA_INT_IRQN                                          (GPIOA_INT_IRQn)
#define PORTA_INT_IIDX                          (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define PORTA_M1_ENCODER_A_IIDX                             (DL_GPIO_IIDX_DIO22)
#define PORTA_M1_ENCODER_A_PIN                                  (DL_GPIO_PIN_22)
#define PORTA_M1_ENCODER_A_IOMUX                                 (IOMUX_PINCM47)
/* Defines for M2_ENCODER_A: GPIOA.24 with pinCMx 54 on package pin 25 */
#define PORTA_M2_ENCODER_A_IIDX                             (DL_GPIO_IIDX_DIO24)
#define PORTA_M2_ENCODER_A_PIN                                  (DL_GPIO_PIN_24)
#define PORTA_M2_ENCODER_A_IOMUX                                 (IOMUX_PINCM54)
/* Defines for M2_ENCODER_B: GPIOA.26 with pinCMx 59 on package pin 30 */
#define PORTA_M2_ENCODER_B_IIDX                             (DL_GPIO_IIDX_DIO26)
#define PORTA_M2_ENCODER_B_PIN                                  (DL_GPIO_PIN_26)
#define PORTA_M2_ENCODER_B_IOMUX                                 (IOMUX_PINCM59)
/* Defines for BUZZER: GPIOA.30 with pinCMx 5 on package pin 37 */
#define PORTA_BUZZER_PIN                                        (DL_GPIO_PIN_30)
#define PORTA_BUZZER_IOMUX                                        (IOMUX_PINCM5)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);
void SYSCFG_DL_Motor_PWM1_init(void);
void SYSCFG_DL_Motor_PWM2_init(void);
void SYSCFG_DL_I2C_0_init(void);
void SYSCFG_DL_GYRO_UART_init(void);
void SYSCFG_DL_CAM_UART_init(void);
void SYSCFG_DL_DEBUG_UART_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
