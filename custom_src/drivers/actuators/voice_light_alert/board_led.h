#ifndef BOARD_LED_H__
#define BOARD_LED_H__

#include <stdbool.h>

/* Tianmengxing onboard B22 LED: PB22 high = on, low = off. */
void board_led_init(void);
void board_led_set(bool enabled);
void board_led_toggle(void);

#endif
