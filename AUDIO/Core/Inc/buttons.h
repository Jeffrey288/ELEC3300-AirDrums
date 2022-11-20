#include "stm32f1xx_hal.h"

extern int hits;

typedef enum {
    BTN_UP,
    BTN_DOWN,
	BTN_HOLD,
} ButtonState;

typedef enum {
    BTN_PRESSED,
    BTN_HELD,
    BTN_RELEASED,
} ButtonEvent;

typedef struct ButtonStruct{
    void (*eventListener)(ButtonEvent);
    int (*stateRetriever)(struct ButtonStruct*);
    uint8_t pin;
    uint32_t last_pressed;
    uint16_t debounce_time;
    uint16_t held_time;
    ButtonState state;
} Button;

extern Button buttons[20];
extern uint16_t numButtons;

void initButtons();

static void addButton(Button button) {
	buttons[numButtons++] = button;
}

static inline void updateButtons() {

	for (int i = 0; i < numButtons; i++) {
		ButtonState tempState = buttons[i].stateRetriever(&buttons[i]) ? BTN_DOWN : BTN_UP;
		if (tempState == BTN_DOWN) {
			buttons[i].last_pressed = HAL_GetTick();
			if (buttons[i].state == BTN_UP) {
				buttons[i].state = BTN_DOWN;
				buttons[i].eventListener(BTN_PRESSED);
			}
		} else {
			if (buttons[i].state == BTN_DOWN || buttons[i].state == BTN_HOLD) {
				if (HAL_GetTick() - buttons[i].last_pressed >= buttons[i].debounce_time) {
					buttons[i].eventListener(BTN_RELEASED);
					buttons[i].state = BTN_UP;
				} else if (HAL_GetTick() - buttons[i].last_pressed >= buttons[i].held_time && buttons[i].state == BTN_DOWN) {
					buttons[i].eventListener(BTN_HELD);
					buttons[i].state = BTN_HOLD;
				}
			}
		}
	}

}
