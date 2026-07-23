#include "motor_hardware.h"
#include "log.h"

static void tb6612_enable_all_motor_impl(const MotorSystemConfig *config);
static void tb6612_disable_all_motor_impl(const MotorSystemConfig *config);
static void tb6612_set_pwms_impl(const MotorSystemConfig *config,
    const int *pwms);

motorHardWareInterface tb6612_interface = {
    .disable_all_motor = tb6612_disable_all_motor_impl,
    .enable_all_motor = tb6612_enable_all_motor_impl,
    .set_pwms = tb6612_set_pwms_impl,
};

static int get_configured_motor_count(const MotorSystemConfig *config)
{
    int count = (int) config->motor_count;

    return count > NUM_MOTORS ? NUM_MOTORS : count;
}

static void motor_set_pwm_compare(const MotorConfig *motor,
    uint32_t compare_value)
{
    if (motor->timer_instance != NULL) {
        DL_Timer_setCaptureCompareValue(motor->timer_instance, compare_value,
            motor->pwm_cc_index);
    }
}

static void motor_coast(const MotorConfig *motor)
{
    motor_set_pwm_compare(motor, 0U);
    if (motor->in1_port != NULL) {
        DL_GPIO_clearPins(motor->in1_port, motor->in1_pin);
    }
    if (motor->in2_port != NULL) {
        DL_GPIO_clearPins(motor->in2_port, motor->in2_pin);
    }
}

static void motor_set_direction(const MotorConfig *motor, bool reverse)
{
    if (reverse) {
        DL_GPIO_clearPins(motor->in1_port, motor->in1_pin);
        DL_GPIO_setPins(motor->in2_port, motor->in2_pin);
    } else {
        DL_GPIO_setPins(motor->in1_port, motor->in1_pin);
        DL_GPIO_clearPins(motor->in2_port, motor->in2_pin);
    }
}

static void tb6612_enable_all_motor_impl(const MotorSystemConfig *config)
{
    int motor_count = get_configured_motor_count(config);

    for (int i = 0; i < motor_count; i++) {
        const MotorConfig *motor = &config->motors[i];

        if (!motor->enabled) {
            continue;
        }
        motor_coast(motor);
        if (motor->timer_instance != NULL) {
            DL_Timer_startCounter(motor->timer_instance);
        }
    }
}

static void tb6612_disable_all_motor_impl(const MotorSystemConfig *config)
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
    }
}

static void tb6612_set_pwms_impl(const MotorSystemConfig *config,
    const int *pwms)
{
    int limited_pwms[NUM_MOTORS] = {0};
    int motor_count;

    if (config == NULL || pwms == NULL) {
        log_e("Invalid parameters in tb6612_set_pwms_impl\n");
        return;
    }

    motor_count = get_configured_motor_count(config);
    for (int i = 0; i < motor_count; i++) {
        const MotorConfig *motor = &config->motors[i];
        int pwm_limit = config->max_pwm_value;

        if (!motor->enabled || motor->timer_instance == NULL ||
            motor->in1_port == NULL || motor->in2_port == NULL ||
            pwm_limit <= 0) {
            continue;
        }
        if ((uint32_t) pwm_limit > DL_Timer_getLoadValue(motor->timer_instance)) {
            pwm_limit = (int) DL_Timer_getLoadValue(motor->timer_instance);
        }
        limited_pwms[i] = amplitude_limit(pwms[i], pwm_limit);
    }

    /* Coast before changing direction, so IN1 and IN2 never overlap. */
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

        motor_set_direction(motor, reverse);
        motor_set_pwm_compare(motor, pwm_value);
    }
}
