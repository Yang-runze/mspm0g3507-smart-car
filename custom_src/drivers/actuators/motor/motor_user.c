#include "motor_user.h"

extern motorHardWareInterface tb6612_interface;

MotorSystemConfig tb6612_cfg = {
    .motor_count = MOTOR_TYPE_TWO_WHEEL,
    .max_pwm_value = MOTOR_PWM_COMMAND_MAX,
    .motors = {
        [MOTOR_FRONT_LEFT] = {
            .enabled = true,
            .timer_instance = (GPTIMER_Regs *) Motor_PWM2_INST,
            .pwm_cc_index = GPIO_Motor_PWM2_C0_IDX,
            .second_timer_instance = (GPTIMER_Regs *) Motor_PWM1_INST,
            .second_pwm_cc_index = GPIO_Motor_PWM1_C1_IDX,
            .polarity = MOTOR_LEFT_DIRECTION_INVERTED,
        },
        [MOTOR_FRONT_RIGHT] = {
            .enabled = true,
            .timer_instance = (GPTIMER_Regs *) Motor_PWM1_INST,
            .pwm_cc_index = GPIO_Motor_PWM1_C2_IDX,
            .second_timer_instance = (GPTIMER_Regs *) Motor_PWM1_INST,
            .second_pwm_cc_index = GPIO_Motor_PWM1_C3_IDX,
            .polarity = MOTOR_RIGHT_DIRECTION_INVERTED,
        },
    },
};

void motor_init(void)
{
    static const int stopped_pwms[MOTOR_TYPE_TWO_WHEEL] = {0, 0};

    tb6612_interface.set_pwms(&tb6612_cfg, stopped_pwms);
    tb6612_interface.enable_all_motor(&tb6612_cfg);
}

void motor_set_pwms(const int *pwms)
{
    tb6612_interface.set_pwms(&tb6612_cfg, pwms);
}

void motor_stop(void)
{
    tb6612_interface.disable_all_motor(&tb6612_cfg);
}
