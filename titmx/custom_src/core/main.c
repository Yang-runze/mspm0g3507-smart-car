#include "ti_msp_dl_config.h"
#include "tests.h"
#include "beep.h"
#include "board_led.h"

static void board_init(void)
{
    SYSCFG_DL_init();
    /* PB3 drives the active-high buzzer enable through ULN2001. */
    beep_init();
    board_led_init();
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
    board_led_set(true);

    while (1) {
    }
}
