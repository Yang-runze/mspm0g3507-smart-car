#include "ti_msp_dl_config.h"
#include "tests.h"
#include "rgb_led.h"

static void board_init(void)
{
    SYSCFG_DL_init();
    /* ULN2001 input is active-high; keep PA30 low so the buzzer stays off. */
    DL_GPIO_clearPins(PORTA_PORT, PORTA_BUZZER_PIN);
    rgb_led_init();
}

int main(void)
{
    board_init();

    module_test_run();

    while (1) {
    }
}

void HardFault_Handler(void)
{
    led_set_color(COLOR_RED);

    while (1) {
    }
}
