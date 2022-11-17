#include "buttons.h"
#include "audio.h"

Button buttons[20];
uint16_t numButtons = 0;

int _drum1Get() { return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == GPIO_PIN_RESET; }
void _drum1Event(ButtonEvent evt) { if (evt == BTN_PRESSED) drumPlay(KICK); }
int _drum2Get() { return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET; }
void _drum2Event(ButtonEvent evt) { if (evt == BTN_PRESSED) drumPlay(CRASH); }
int _drum3Get() { return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_10) == GPIO_PIN_RESET; }
void _drum3Event(ButtonEvent evt) { if (evt == BTN_PRESSED) drumPlay(LOW_TOM); }
int _drum4Get() { return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_11) == GPIO_PIN_RESET; }
void _drum4Event(ButtonEvent evt) { if (evt == BTN_PRESSED) drumPlay(HIGH_TOM); }

/**
	void (*eventListener)(int);
	int (*stateRetriever)();
	uint32_t last_pressed;
	uint16_t debounce_time;
	ButtonState state;
 */


void initButtons() {
	addButton((Button) {
		.eventListener = _drum1Event,
		.stateRetriever = _drum1Get,
		.last_pressed = 0,
		.debounce_time = 10,
		.state = BTN_UP,
	});
	addButton((Button) {
		.eventListener = _drum2Event,
		.stateRetriever = _drum2Get,
		.last_pressed = 0,
		.debounce_time = 10,
		.state = BTN_UP,
	});
	addButton((Button) {
		.eventListener = _drum3Event,
		.stateRetriever = _drum3Get,
		.last_pressed = 0,
		.debounce_time = 10,
		.state = BTN_UP,
	});
	addButton((Button) {
		.eventListener = _drum4Event,
		.stateRetriever = _drum4Get,
		.last_pressed = 0,
		.debounce_time = 10,
		.state = BTN_UP,
	});
}
