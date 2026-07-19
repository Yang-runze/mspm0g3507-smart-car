#include "servo_user.h"

#include "ti_msp_dl_config.h"

#define SERVO_MIN_PULSE_US     500U
#define SERVO_MAX_PULSE_US    2500U
#define SERVO_CENTER_PULSE_US 1500U

static uint32_t servo_pulse_us_to_ticks(uint16_t pulse_us)
{
    if (pulse_us < SERVO_MIN_PULSE_US) {
        pulse_us = SERVO_MIN_PULSE_US;
    } else if (pulse_us > SERVO_MAX_PULSE_US) {
        pulse_us = SERVO_MAX_PULSE_US;
    }

    return ((uint32_t) pulse_us * 5U) / 2U;
}

void servo_init(void)
{
    servo_set_pulse_us(SERVO_CHANNEL_0, SERVO_CENTER_PULSE_US);
    servo_set_pulse_us(SERVO_CHANNEL_1, SERVO_CENTER_PULSE_US);
}

void servo_set_pulse_us(uint8_t channel, uint16_t pulse_us)
{
    uint32_t ticks = servo_pulse_us_to_ticks(pulse_us);

    if (channel == SERVO_CHANNEL_0) {
        DL_TimerG_setCaptureCompareValue(
            SERVO_PWM_INST, ticks, DL_TIMER_CC_0_INDEX);
    } else if (channel == SERVO_CHANNEL_1) {
        DL_TimerG_setCaptureCompareValue(
            SERVO_PWM_INST, ticks, DL_TIMER_CC_1_INDEX);
    }
}
