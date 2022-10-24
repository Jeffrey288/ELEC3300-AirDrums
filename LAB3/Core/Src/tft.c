#include "tft.h"
#include "lcd.h"
#include "tft_helpers.h"
#include "audio.h"

int errorFlag = 0;
char errorMsg[30];

void error_handler(char err[30]) {
	errorFlag = 1;
	char* pt;
	for (pt = errorMsg; *err != 0; err++, pt++) {
		*pt = *err;
	} *pt = 0; // ending the string
	HAL_Delay(1000);
}

void tft_task() { // help other threads print their stuff
	LCD_INIT();
	int last_tick = HAL_GetTick();
	char buff[100];

	while (1) {

		if (HAL_GetTick() - last_tick > 100) {
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
			last_tick = HAL_GetTick();
			sprintf(buff, "%02d:%02d.%03d",
					HAL_GetTick() / (1000 * 60),
					(HAL_GetTick() / 1000) % (60),
					HAL_GetTick() % 1000);
			LCD_DrawString(0, 0, buff);
//			tft_prints(0, 0, "%02d:%20d.%03d",
//					HAL_GetTick() / (1000 * 60),
//					(HAL_GetTick() / 1000) % (60),
//					HAL_GetTick() % 1000);
//			audio_tft();
		} else {
			vTaskDelay(20);
		}

//		HAL_Delay(500);

	}
	while (1) {
		tft_set_textColor(RED);
		tft_prints(0, 0, errorMsg);
	}
}
