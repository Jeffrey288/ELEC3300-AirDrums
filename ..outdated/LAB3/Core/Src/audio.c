#include "audio.h"
#include "rina.h"
#include "stm32f1xx_hal.h"
#include "tft.h"

#define bit_set(var,bitno) ((var) |= 1 << (bitno))
#define bit_clr(var,bitno) ((var) &= ~(1 << (bitno)))
#define testbit(var,bitno) (((var)>>(bitno)) & 0x01)
#define minmax(val,minval,maxval) \
	if ((val) > (maxval)) (val) = (maxval); \
	else if ((val) < (minval)) (val) = (minval);

int16_t sample_sum;
uint16_t dac_out;
int i = 0;

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;

void audio_tft() {
	tft_prints(0, 1, "sample: %d", i);
}

void audio_task() {

	// Starting TIM2
	//  TIM2->ARR = 72000000 / 44100 - 1;
	//  TIM2->ARR = 72000000 / 22050 - 1;
	TIM2->ARR = 72000000 / 22050 - 1;
	HAL_TIM_Base_Start(&htim2);

	int last_tick = HAL_GetTick();

	while (1) {

		if (HAL_GetTick() - last_tick > 200) {
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
			last_tick = HAL_GetTick();
		}

//		sample_sum = rina[i];
		sample_sum = 0;
		minmax(sample_sum, -32768, 32767);
		sample_sum += 32768;
		dac_out = sample_sum;
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (dac_out) >> 4);
		i = (i + 1) % MUSIC_LENGTH;

		while (testbit(TIM2->SR, 0) == 0) {
			vTaskDelay(1); /* wait for timer reset */
		}
		bit_clr(TIM2->SR, 0);
	}
}
