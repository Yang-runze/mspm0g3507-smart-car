#include "board_led.h"
#include "ti_msp_dl_config.h"

void board_led_init(void)
{
    board_led_set(false);
}

void board_led_set(bool enabled)
{
    if (enabled) {
        DL_GPIO_setPins(PORTB_PORT, PORTB_BOARD_LED_PIN);
    } else {
        DL_GPIO_clearPins(PORTB_PORT, PORTB_BOARD_LED_PIN);
    }
}

void board_led_toggle(void)
{
    DL_GPIO_togglePins(PORTB_PORT, PORTB_BOARD_LED_PIN);
}
