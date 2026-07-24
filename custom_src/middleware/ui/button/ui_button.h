#ifndef BUTTON_APP_H__
#define BUTTON_APP_H__

#include <stdbool.h>
#include "multi_button.h"

#define BUTTON_NUM 4

typedef enum {
	BUTTON_UP = 0,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
} BUTTON_ID;

void user_button_init(BtnCallback single_click_cb, BtnCallback long_press_cb);
bool user_button_is_pressed(BUTTON_ID button_id);

extern struct Button buttons[BUTTON_NUM];

#endif
