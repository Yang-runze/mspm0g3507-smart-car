/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerG_backupConfig gMotor_PWM1Backup;
DL_UART_Main_backupConfig gDEBUG_UARTBackup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_Motor_PWM1_init();
    SYSCFG_DL_Motor_PWM2_init();
    SYSCFG_DL_GYRO_UART_init();
    SYSCFG_DL_CAM_UART_init();
    SYSCFG_DL_DEBUG_UART_init();
    SYSCFG_DL_GIMBAL_UART_init();
    /* Ensure backup structures have no valid state */
	gMotor_PWM1Backup.backupRdy 	= false;
	gDEBUG_UARTBackup.backupRdy 	= false;

}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerG_saveConfiguration(Motor_PWM1_INST, &gMotor_PWM1Backup);
	retStatus &= DL_UART_Main_saveConfiguration(DEBUG_UART_INST, &gDEBUG_UARTBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerG_restoreConfiguration(Motor_PWM1_INST, &gMotor_PWM1Backup, false);
	retStatus &= DL_UART_Main_restoreConfiguration(DEBUG_UART_INST, &gDEBUG_UARTBackup);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerG_reset(Motor_PWM1_INST);
    DL_TimerG_reset(Motor_PWM2_INST);
    DL_UART_Main_reset(GYRO_UART_INST);
    DL_UART_Main_reset(CAM_UART_INST);
    DL_UART_Main_reset(DEBUG_UART_INST);
    DL_UART_Main_reset(GIMBAL_UART_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerG_enablePower(Motor_PWM1_INST);
    DL_TimerG_enablePower(Motor_PWM2_INST);
    DL_UART_Main_enablePower(GYRO_UART_INST);
    DL_UART_Main_enablePower(CAM_UART_INST);
    DL_UART_Main_enablePower(DEBUG_UART_INST);
    DL_UART_Main_enablePower(GIMBAL_UART_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralOutputFunction(GPIO_Motor_PWM1_C0_IOMUX,GPIO_Motor_PWM1_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_Motor_PWM1_C0_PORT, GPIO_Motor_PWM1_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_Motor_PWM2_C1_IOMUX,GPIO_Motor_PWM2_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_Motor_PWM2_C1_PORT, GPIO_Motor_PWM2_C1_PIN);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_GYRO_UART_IOMUX_TX, GPIO_GYRO_UART_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_GYRO_UART_IOMUX_RX, GPIO_GYRO_UART_IOMUX_RX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_CAM_UART_IOMUX_TX, GPIO_CAM_UART_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_CAM_UART_IOMUX_RX, GPIO_CAM_UART_IOMUX_RX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_DEBUG_UART_IOMUX_TX, GPIO_DEBUG_UART_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_DEBUG_UART_IOMUX_RX, GPIO_DEBUG_UART_IOMUX_RX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_GIMBAL_UART_IOMUX_TX, GPIO_GIMBAL_UART_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_GIMBAL_UART_IOMUX_RX, GPIO_GIMBAL_UART_IOMUX_RX_FUNC);

    DL_GPIO_initDigitalOutput(PORTB_TRACK_CLK_IOMUX);

    DL_GPIO_initDigitalInputFeatures(PORTB_TRACK_DAT_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(PORTB_M1_IN2_IOMUX);

    DL_GPIO_initDigitalOutput(PORTB_M1_IN1_IOMUX);

    DL_GPIO_initDigitalInputFeatures(PORTB_M1_ENCODER_A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(PORTB_M1_ENCODER_B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(PORTB_M2_ENCODER_B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(PORTB_BUZZER_IOMUX);

    DL_GPIO_initDigitalInputFeatures(PORTB_KEY3_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(PORTB_KEY4_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutputFeatures(PORTB_BOARD_LED_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);

    DL_GPIO_initDigitalOutput(PORTA_M2_IN1_IOMUX);

    DL_GPIO_initDigitalOutput(PORTA_M2_IN2_IOMUX);

    DL_GPIO_initDigitalInputFeatures(PORTA_M2_ENCODER_A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(PORTA_KEY1_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(PORTA_KEY2_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(PORTA_OLED_SDA_IOMUX);

    DL_GPIO_initDigitalOutput(PORTA_OLED_SCL_IOMUX);

    DL_GPIO_clearPins(PORTA_PORT, PORTA_M2_IN1_PIN |
		PORTA_M2_IN2_PIN |
		PORTA_OLED_SDA_PIN |
		PORTA_OLED_SCL_PIN);
    DL_GPIO_enableOutput(PORTA_PORT, PORTA_M2_IN1_PIN |
		PORTA_M2_IN2_PIN |
		PORTA_OLED_SDA_PIN |
		PORTA_OLED_SCL_PIN);
    DL_GPIO_setUpperPinsPolarity(PORTA_PORT, DL_GPIO_PIN_29_EDGE_RISE_FALL);
    DL_GPIO_clearInterruptStatus(PORTA_PORT, PORTA_M2_ENCODER_A_PIN);
    DL_GPIO_enableInterrupt(PORTA_PORT, PORTA_M2_ENCODER_A_PIN);
    DL_GPIO_clearPins(PORTB_PORT, PORTB_TRACK_CLK_PIN |
		PORTB_M1_IN2_PIN |
		PORTB_M1_IN1_PIN |
		PORTB_BUZZER_PIN |
		PORTB_BOARD_LED_PIN);
    DL_GPIO_enableOutput(PORTB_PORT, PORTB_TRACK_CLK_PIN |
		PORTB_M1_IN2_PIN |
		PORTB_M1_IN1_PIN |
		PORTB_BUZZER_PIN |
		PORTB_BOARD_LED_PIN);
    DL_GPIO_setLowerPinsPolarity(PORTB_PORT, DL_GPIO_PIN_13_EDGE_RISE_FALL |
		DL_GPIO_PIN_12_EDGE_RISE_FALL);
    DL_GPIO_setUpperPinsPolarity(PORTB_PORT, DL_GPIO_PIN_26_EDGE_RISE_FALL);
    DL_GPIO_clearInterruptStatus(PORTB_PORT, PORTB_M1_ENCODER_A_PIN |
		PORTB_M1_ENCODER_B_PIN |
		PORTB_M2_ENCODER_B_PIN);
    DL_GPIO_enableInterrupt(PORTB_PORT, PORTB_M1_ENCODER_A_PIN |
		PORTB_M1_ENCODER_B_PIN |
		PORTB_M2_ENCODER_B_PIN);

}


static const DL_SYSCTL_SYSPLLConfig gSYSPLLConfig = {
    .inputFreq              = DL_SYSCTL_SYSPLL_INPUT_FREQ_16_32_MHZ,
	.rDivClk2x              = 3,
	.rDivClk1               = 0,
	.rDivClk0               = 0,
	.enableCLK2x            = DL_SYSCTL_SYSPLL_CLK2X_ENABLE,
	.enableCLK1             = DL_SYSCTL_SYSPLL_CLK1_DISABLE,
	.enableCLK0             = DL_SYSCTL_SYSPLL_CLK0_DISABLE,
	.sysPLLMCLK             = DL_SYSCTL_SYSPLL_MCLK_CLK2X,
	.sysPLLRef              = DL_SYSCTL_SYSPLL_REF_SYSOSC,
	.qDiv                   = 9,
	.pDiv                   = DL_SYSCTL_SYSPLL_PDIV_2
};

SYSCONFIG_WEAK bool SYSCFG_DL_SYSCTL_SYSPLL_init(void)
{
    bool fFCCRatioStatus = false;
    uint32_t fFCCSysoscCount;
    uint32_t fFCCPllCount;
    uint32_t fFCCRatio;
    uint32_t fccTimeOutCounter;

    DL_SYSCTL_setFCCPeriods( DL_SYSCTL_FCC_TRIG_CNT_01 );

    /* Measuring PLL. */
    DL_SYSCTL_configFCC(DL_SYSCTL_FCC_TRIG_TYPE_RISE_RISE,
                        DL_SYSCTL_FCC_TRIG_SOURCE_LFCLK,
                        DL_SYSCTL_FCC_CLOCK_SOURCE_SYSPLLCLK2X);
    /* Get SYSPLL frequency using FCC */
    fccTimeOutCounter = 0;
    DL_SYSCTL_startFCC();
    while (DL_SYSCTL_isFCCDone() == 0) {
        delay_cycles(977);  /* 1x LFCLK cycle = 32MHz/32.768kHz = 977, 30.5us */
        fccTimeOutCounter++;
        if(fccTimeOutCounter > 65){
            /* Timeout set to approximately 2ms (user-customizable) */
            break;
        }
    }

    /* get measA= SYSPLLCLK2X freq wrt LFOSC*/
    fFCCPllCount = DL_SYSCTL_readFCC();

    /* Measuring SYSPLL Source */
    DL_SYSCTL_configFCC(DL_SYSCTL_FCC_TRIG_TYPE_RISE_RISE,
                        DL_SYSCTL_FCC_TRIG_SOURCE_LFCLK,
                        DL_SYSCTL_FCC_CLOCK_SOURCE_SYSOSC);
    /* Get SYSPLL frequency using FCC */
    fccTimeOutCounter = 0;
    DL_SYSCTL_startFCC();
    while (DL_SYSCTL_isFCCDone() == 0) {
        delay_cycles(977);  /* 1x LFCLK cycle = 32MHz/32.768kHz = 977, 30.5us */
        fccTimeOutCounter++;
        if(fccTimeOutCounter > 65){
            /* Timeout set to approximately 2ms (user-customizable) */
            break;
        }
    }

    /* get measB= SYSOSC freq wrt LFOSC*/
    fFCCSysoscCount = DL_SYSCTL_readFCC();

    /* Get ratio of both measurements*/
    fFCCRatio = (fFCCPllCount * FLOAT_TO_INT_SCALE) / fFCCSysoscCount;
    /* Check ratio is within bounds*/
    if ((FCC_LOWER_BOUND <  fFCCRatio) && (fFCCRatio < FCC_UPPER_BOUND))
    {
        /* ratio is good for proceeding into application code. */
        fFCCRatioStatus = true;
    }

    return fFCCRatioStatus;
}
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setFlashWaitState(DL_SYSCTL_FLASH_WAIT_STATE_2);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_configSYSPLL((DL_SYSCTL_SYSPLLConfig *) &gSYSPLLConfig);

    /*
     * [SYSPLL_ERR_01]
     * PLL Incorrect locking WA start.
     * Insert after every PLL enable.
     * This can lead an infinite loop if the condition persists
     * and can block entry to the application code.
     */

    while (SYSCFG_DL_SYSCTL_SYSPLL_init() == false)
    {
        /* Toggle SYSPLL enable to re-enable SYSPLL and re-check incorrect locking */
        DL_SYSCTL_disableSYSPLL();
        DL_SYSCTL_enableSYSPLL();

        /* Wait until SYSPLL startup is stabilized*/
        while ((DL_SYSCTL_getClockStatus() & SYSCTL_CLKSTATUS_SYSPLLGOOD_MASK) != DL_SYSCTL_CLK_STATUS_SYSPLL_GOOD){}
    }
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_2);
    DL_SYSCTL_enableMFCLK();
    DL_SYSCTL_setMCLKSource(SYSOSC, HSCLK, DL_SYSCTL_HSCLK_SOURCE_SYSPLL);

}


/*
 * Timer clock configuration to be sourced by  / 8 (10000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   10000000 Hz = 10000000 Hz / (8 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gMotor_PWM1ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale = 0U
};

static const DL_TimerG_PWMConfig gMotor_PWM1Config = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
    .period = 3000,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_START,
};

SYSCONFIG_WEAK void SYSCFG_DL_Motor_PWM1_init(void) {

    DL_TimerG_setClockConfig(
        Motor_PWM1_INST, (DL_TimerG_ClockConfig *) &gMotor_PWM1ClockConfig);

    DL_TimerG_initPWMMode(
        Motor_PWM1_INST, (DL_TimerG_PWMConfig *) &gMotor_PWM1Config);

    // Set Counter control to the smallest CC index being used
    DL_TimerG_setCounterControl(Motor_PWM1_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerG_setCaptureCompareOutCtl(Motor_PWM1_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_0_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(Motor_PWM1_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareValue(Motor_PWM1_INST, 0, DL_TIMER_CC_0_INDEX);

    DL_TimerG_enableClock(Motor_PWM1_INST);


    
    DL_TimerG_setCCPDirection(Motor_PWM1_INST , DL_TIMER_CC0_OUTPUT );


}
/*
 * Timer clock configuration to be sourced by  / 4 (10000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   10000000 Hz = 10000000 Hz / (4 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gMotor_PWM2ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_4,
    .prescale = 0U
};

static const DL_TimerG_PWMConfig gMotor_PWM2Config = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
    .period = 3000,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_START,
};

SYSCONFIG_WEAK void SYSCFG_DL_Motor_PWM2_init(void) {

    DL_TimerG_setClockConfig(
        Motor_PWM2_INST, (DL_TimerG_ClockConfig *) &gMotor_PWM2ClockConfig);

    DL_TimerG_initPWMMode(
        Motor_PWM2_INST, (DL_TimerG_PWMConfig *) &gMotor_PWM2Config);

    // Set Counter control to the smallest CC index being used
    DL_TimerG_setCounterControl(Motor_PWM2_INST,DL_TIMER_CZC_CCCTL1_ZCOND,DL_TIMER_CAC_CCCTL1_ACOND,DL_TIMER_CLC_CCCTL1_LCOND);

    DL_TimerG_setCaptureCompareOutCtl(Motor_PWM2_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_1_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(Motor_PWM2_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    DL_TimerG_setCaptureCompareValue(Motor_PWM2_INST, 0, DL_TIMER_CC_1_INDEX);

    DL_TimerG_enableClock(Motor_PWM2_INST);


    
    DL_TimerG_setCCPDirection(Motor_PWM2_INST , DL_TIMER_CC1_OUTPUT );


}


static const DL_UART_Main_ClockConfig gGYRO_UARTClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_MFCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gGYRO_UARTConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_GYRO_UART_init(void)
{
    DL_UART_Main_setClockConfig(GYRO_UART_INST, (DL_UART_Main_ClockConfig *) &gGYRO_UARTClockConfig);

    DL_UART_Main_init(GYRO_UART_INST, (DL_UART_Main_Config *) &gGYRO_UARTConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115107.91
     */
    DL_UART_Main_setOversampling(GYRO_UART_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(GYRO_UART_INST, GYRO_UART_IBRD_4_MHZ_115200_BAUD, GYRO_UART_FBRD_4_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(GYRO_UART_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);


    DL_UART_Main_enable(GYRO_UART_INST);
}
static const DL_UART_Main_ClockConfig gCAM_UARTClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gCAM_UARTConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_CAM_UART_init(void)
{
    DL_UART_Main_setClockConfig(CAM_UART_INST, (DL_UART_Main_ClockConfig *) &gCAM_UARTClockConfig);

    DL_UART_Main_init(CAM_UART_INST, (DL_UART_Main_Config *) &gCAM_UARTConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(CAM_UART_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(CAM_UART_INST, CAM_UART_IBRD_40_MHZ_115200_BAUD, CAM_UART_FBRD_40_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(CAM_UART_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);


    DL_UART_Main_enable(CAM_UART_INST);
}
static const DL_UART_Main_ClockConfig gDEBUG_UARTClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gDEBUG_UARTConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_DEBUG_UART_init(void)
{
    DL_UART_Main_setClockConfig(DEBUG_UART_INST, (DL_UART_Main_ClockConfig *) &gDEBUG_UARTClockConfig);

    DL_UART_Main_init(DEBUG_UART_INST, (DL_UART_Main_Config *) &gDEBUG_UARTConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(DEBUG_UART_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(DEBUG_UART_INST, DEBUG_UART_IBRD_80_MHZ_115200_BAUD, DEBUG_UART_FBRD_80_MHZ_115200_BAUD);



    DL_UART_Main_enable(DEBUG_UART_INST);
}
static const DL_UART_Main_ClockConfig gGIMBAL_UARTClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gGIMBAL_UARTConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_GIMBAL_UART_init(void)
{
    DL_UART_Main_setClockConfig(GIMBAL_UART_INST, (DL_UART_Main_ClockConfig *) &gGIMBAL_UARTClockConfig);

    DL_UART_Main_init(GIMBAL_UART_INST, (DL_UART_Main_Config *) &gGIMBAL_UARTConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(GIMBAL_UART_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(GIMBAL_UART_INST, GIMBAL_UART_IBRD_40_MHZ_115200_BAUD, GIMBAL_UART_FBRD_40_MHZ_115200_BAUD);



    DL_UART_Main_enable(GIMBAL_UART_INST);
}

