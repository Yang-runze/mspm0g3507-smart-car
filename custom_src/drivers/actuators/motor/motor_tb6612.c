#include "motor_hardware.h"
#include "log.h"

static void drv8870_enable_all_motor_impl(const MotorSystemConfig *config);
static void drv8870_disable_all_motor_impl(const MotorSystemConfig *config);
static void drv8870_set_pwms_impl(const MotorSystemConfig *config,
    const int *pwms);

/* Keep the exported name for compatibility with the existing application. */
motorHardWareInterface tb6612_interface = {
    .disable_all_motor = drv8870_disable_all_motor_impl,
    .enable_all_motor = drv8870_enable_all_motor_impl,
    .set_pwms = drv8870_set_pwms_impl,
};

static int get_configured_motor_count(const MotorSystemConfig *config)
{
    int count = (int) config->motor_count;

    return count > NUM_MOTORS ? NUM_MOTORS : count;
}

static void motor_set_input_compare(const MotorConfig *motor,
    bool second_input, uint32_t compare_value)
{
    GPTIMER_Regs *timer = second_input ? motor->second_timer_instance :
        motor->timer_instance;
    uint32_t channel = second_input ? motor->second_pwm_cc_index :
        motor->pwm_cc_index;

    if (timer != NULL) {
        DL_Timer_setCaptureCompareValue(timer, compare_value, channel);
    }
}

static void motor_coast(const MotorConfig *motor)
{
    motor_set_input_compare(motor, false, 0U);
    motor_set_input_compare(motor, true, 0U);
}

static void drv8870_enable_all_motor_impl(const MotorSystemConfig *config)
{
    int motor_count = get_configured_motor_count(config);

    for (int i = 0; i < motor_count; i++) {
        const MotorConfig *motor = &config->motors[i];

        if (!motor->enabled) {
            continue;
        }
        if (motor->timer_instance != NULL) {
            DL_Timer_startCounter(motor->timer_instance);
        }
        if (motor->second_timer_instance != NULL &&
            motor->second_timer_instance != motor->timer_instance) {
            DL_Timer_startCounter(motor->second_timer_instance);
        }
    }
}

static void drv8870_disable_all_motor_impl(const MotorSystemConfig *config)
{
    int motor_count = get_configured_motor_count(config);

    for (int i = 0; i < motor_count; i++) {
        const MotorConfig *motor = &config->motors[i];

        if (!motor->enabled) {
            continue;
        }
        motor_coast(motor);
        if (motor->timer_instance != NULL) {
            DL_Timer_stopCounter(motor->timer_instance);
        }
        if (motor->second_timer_instance != NULL &&
            motor->second_timer_instance != motor->timer_instance) {
            DL_Timer_stopCounter(motor->second_timer_instance);
        }
    }
}

static void drv8870_set_pwms_impl(const MotorSystemConfig *config,
    const int *pwms)
{
    int limited_pwms[NUM_MOTORS] = {0};
    int motor_count;

    if (config == NULL || pwms == NULL) {
        log_e("Invalid parameters in drv8870_set_pwms_impl\n");
        return;
    }

    motor_count = get_configured_motor_count(config);
    for (int i = 0; i < motor_count; i++) {
        const MotorConfig *motor = &config->motors[i];
        int pwm_limit = config->max_pwm_value;

        if (!motor->enabled || motor->timer_instance == NULL ||
            motor->second_timer_instance == NULL || pwm_limit <= 0) {
            continue;
        }
        if ((uint32_t) pwm_limit > DL_Timer_getLoadValue(motor->timer_instance)) {
            pwm_limit = (int) DL_Timer_getLoadValue(motor->timer_instance);
        }
        limited_pwms[i] = amplitude_limit(pwms[i], pwm_limit);
    }

    /* Coast before selecting the opposite input, so IN1/IN2 never overlap. */
    for (int i = 0; i < motor_count; i++) {
        motor_coast(&config->motors[i]);
    }

    for (int i = 0; i < motor_count; i++) {
        const MotorConfig *motor = &config->motors[i];
        int limited_pwm = limited_pwms[i];
        bool reverse = limited_pwm < 0;
        uint32_t pwm_value = reverse ? (uint32_t) (-limited_pwm) :
            (uint32_t) limited_pwm;

        if (!motor->enabled || pwm_value == 0U) {
            continue;
        }
        if (motor->polarity) {
            reverse = !reverse;
        }

        /* IN1 PWM/IN2 low is forward; swap the active input for reverse. */
        motor_set_input_compare(motor, reverse, pwm_value);
    }
}
