#include "buttons.h"
#include "audio.h"
#include "imu.h"

Button buttons[20];
uint16_t numButtons = 0;

int hits = 0;

int _drum1Get(Button *btn) { return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == GPIO_PIN_RESET; }
void _drum1Event(ButtonEvent evt) { if (evt == BTN_PRESSED) drumPlay(KICK); }
int _drum2Get(Button *btn) { return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET; }
void _drum2Event(ButtonEvent evt) { if (evt == BTN_PRESSED) drumPlay(CRASH); }
int _drum3Get(Button *btn) { return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_10) == GPIO_PIN_RESET; }
void _drum3Event(ButtonEvent evt) { if (evt == BTN_PRESSED) drumPlay(LOW_TOM); }
int _drum4Get(Button *btn) { return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_11) == GPIO_PIN_RESET; }
void _drum4Event(ButtonEvent evt) { if (evt == BTN_PRESSED) drumPlay(HIGH_TOM); }

//typedef {
//	Dow
//} DrumState;
void _drumIMULeftEvent(ButtonEvent evt) {
	if (evt == BTN_PRESSED) {
		hits++;
		if (imuLeft.yaw < -60) drumPlay(0);
		else if (imuLeft.yaw < 0) drumPlay(1);
		else if (imuLeft.yaw < 60) drumPlay(2);
		else drumPlay(3);
	}
}

void _drumIMURightEvent(ButtonEvent evt) {
	if (evt == BTN_PRESSED) {
		hits++;
		if (imuRight.yaw < -100) drumPlay(hits % DRUM_NUM);
		else if (imuRight.yaw < 0) drumPlay(hits % DRUM_NUM);
		else if (imuRight.yaw < 100) drumPlay(hits % DRUM_NUM);
		else drumPlay(hits % DRUM_NUM);
	}
}

int _drumIMULeftGet(Button *btn) {
	if (imuLeft.accelFlag) {
		imuLeft.accelFlag = 0;
		return 1;
	} else {
		return 0;
	}
}

int _drumIMURightGet(Button *btn) {
	if (imuRight.accelFlag) {
		imuRight.accelFlag = 0;
		return 1;
	} else {
		return 0;
	}
}

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
	addButton((Button) {
		.eventListener = _drumIMULeftEvent,
		.stateRetriever = _drumIMULeftGet,
		.last_pressed = 0,
		.debounce_time = 70, // > 25.3 https://www.guinnessworldrecords.com/news/2022/1/worlds-fastest-drummer-11-year-old-achieves-2-370-bpm-688980#:~:text=11%2Dyear%2Dold%20Pritish%20A%20R,that's%2039.5%20beats%20per%20second.
		.state = BTN_UP,
	});
	addButton((Button) {
		.eventListener = _drumIMURightEvent,
		.stateRetriever = _drumIMURightGet,
		.last_pressed = 0,
		.debounce_time = 70, // > 25.3 https://www.guinnessworldrecords.com/news/2022/1/worlds-fastest-drummer-11-year-old-achieves-2-370-bpm-688980#:~:text=11%2Dyear%2Dold%20Pritish%20A%20R,that's%2039.5%20beats%20per%20second.
		.state = BTN_UP,
	});
}
