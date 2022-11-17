#include "stm32f1xx_hal.h"

typedef enum {
    BTN_UP,
    BTN_DOWN,
} ButtonState;

typedef enum {
    BTN_PRESSED,
    BTN_HELD,
    BTN_RELEASED,
} ButtonEvent;

typedef struct {
    void (*eventListener)(ButtonEvent);
    int (*stateRetriever)();
    uint8_t pin;
    uint32_t last_pressed;
    uint16_t debounce_time;
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
		ButtonState tempState = buttons[i].stateRetriever() ? BTN_DOWN : BTN_UP;
		if (tempState == BTN_DOWN) {
			buttons[i].last_pressed = HAL_GetTick();
			if (buttons[i].state == BTN_UP) {
				buttons[i].state = BTN_DOWN;
				buttons[i].eventListener(BTN_PRESSED);
			} else {
				buttons[i].eventListener(BTN_HELD);
			}
		} else {
			if (buttons[i].state == BTN_DOWN) {
				if (HAL_GetTick() - buttons[i].last_pressed > buttons[i].debounce_time) {
					buttons[i].eventListener(BTN_RELEASED);
					buttons[i].state = BTN_UP;
				} else {
					buttons[i].eventListener(BTN_HELD);
				}
			}
		}
	}

}
