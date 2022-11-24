/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "fatfs.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "rina.h"
#include "wav.h"
#include "fileBuff.h"
#include "audio.h"
#include "MPU9250.h"
#include "MPU9250_Config.h"
#include "imu.h"
#include "buttons.h"
#include "kalman.h"
#include "string.h"

// GUI includes
#include <stdio.h>

#include "bsp_ili9341_lcd.h"
#include "bsp_xpt2046_lcd.h"
#include "gui.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define USEMUSIC
#define TESTMUSIC
#define IVANCODE
//#define USEIMU

#ifdef TESTMUSIC
#define USEMUSIC
#endif

void error_handler(int res, const char *msg) {
	char buff[100];
	sprintf(buff, "%d %s", res, msg);
	LCD_DrawString(40, 0, buff);
	HAL_Delay(1000000);
}
void display_success(int num, const char *msg) {
	char buff[100];
	sprintf(buff, "%d %s", num, msg);
	LCD_DrawString(40, 0, buff);
}

char buff[31];
//extern int numActiveDrums;

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
// https://community.st.com/s/article/how-to-create-a-file-system-on-a-sd-card-using-stm32bubeide
	//https://github.com/spanceac/electro-drums/blob/master/drums.c
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_FSMC_Init();
	MX_DAC_Init();
	MX_TIM2_Init();
	MX_FATFS_Init();
	MX_SPI2_Init();
	MX_TIM4_Init();
	MX_SPI1_Init();
	MX_TIM3_Init();
	MX_ADC1_Init();
	/* USER CODE BEGIN 2 */

#ifdef IVANCODE
	ILI9341_Init();
	ILI9341_GramScan(3);
	LCD_SetBackColor(WHITE);
	ILI9341_Clear(0, 0, 320, 240);
#else
	LCD_INIT();
#endif

#ifdef USEIMU
	imu_setActive(&imuLeft);
	MPU9250_Init();
//	imu_setActive(&imuRight);
//	MPU9250_Init();
#endif

#ifdef USEMUSIC
	FRESULT res;
	FATFS FatFs;
	FATFS BlankFs;
	DIR dir;
	FILINFO fno;

	// MOUNT THE SD CARD
	HAL_Delay(20);
	uint32_t sd_card_mount_led_last_tick = HAL_GetTick();
	while (1) {
		if (HAL_GetTick() - sd_card_mount_led_last_tick > 100) {
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
			sd_card_mount_led_last_tick = HAL_GetTick();
		}
		res = f_mount(&FatFs, "", 1);
		if (res != FR_OK) {
			sprintf(buff, "%d No sd card found!", res);
			LCD_DrawString(40, 0, buff);
			continue;
		}
		HAL_Delay(20);

		res = f_opendir(&dir, "0:");
		if (res != FR_OK) {
			sprintf(buff, "%d Cannot open drive!", res);
			LCD_DrawString(40, 0, buff);
			continue;
		}
		HAL_Delay(20);

		break;
	}
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);

	// LIST ALL OF THE AVAILABLE FILES
	while (1) {
		res = f_readdir(&dir, &fno);
		if (res != FR_OK || fno.fname[0] == 0)
			break;
		int len = strlen(fno.fname);
		if (drumMatch(fno.fname) == -1
				&& (fno.fname[len - 1] == 'V' || fno.fname[len - 1] == 'v')) {
			addMusic(fno.fname);
		}
	}

	for (int i = 0; i < musicFileNum; i++) {
		LCD_DrawString(0, 20 + 20 * i, musicFilenames[i]);
	}
#endif

#ifdef USEMUSIC
	audioChannelInit();
#endif

	initButtons();

#ifdef IVANCODE
	int _touchScreenGet() {
		return (XPT2046_TouchDetect() == TOUCH_PRESSED);
	}
	void _touchScreenEvent(ButtonEvent evt) {
		if (evt == BTN_PRESSED) {
			XPT2046_Touch(posinfo); // Get TouchScreen position
			InterfaceSelector(posinfo[0], posinfo[1], GUISTACK[0]); // process the touchscreen position
			if (currentinterface != GUISTACK[0]) { // execute only when the interface is changed
				DisplayInterface(GUISTACK[0]);
				currentinterface = GUISTACK[0];
			}
		}
	}
	addButton((Button) {
		.eventListener = _touchScreenEvent,
		.stateRetriever = _touchScreenGet,
		.last_pressed = 0,
		.debounce_time = 10,
		.state = BTN_UP,
	});
#endif

#ifdef USEIMU
//	imu_calibrateGyro(&imuLeft);
//	imu_calibrateGyro(&imuRight);
	TIM3->PSC = (72000000 / 72000) - 1;
	TIM3->ARR = (72000 / 200) - 1;
	HAL_TIM_Base_Start(&htim3);
	__HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);
#endif

#ifdef IVANCODE
	StartLogo(); // displays logo screen
	while (StartProcess())
		// waiting for the first click
		;// Wait starting signal
	LCD_Clear(0, 0, 320, 240, WHITE);
	LCD_SetBackColor(WHITE);

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

	MainMenuInterface();
#endif

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	int last_tick = HAL_GetTick();

	ButtonState PA0 = BTN_UP;
	ButtonState PC13 = BTN_UP;
	ButtonState PC6 = BTN_UP;
	ButtonState PC7 = BTN_UP;
	int musicNum = 0; // temporary variable for testing purposes

	while (1) {

		if (HAL_GetTick() - last_tick > 200) {
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
			last_tick = HAL_GetTick();
		}

#ifdef USEIMU

		sprintf(buff, "hit count: %3d %3d %3d ", hits, imuLeft.upCount, imuLeft.downCount);
		LCD_DrawString(0, 140, buff);
//		LCD_DrawFormattedString(0, 140, "count: %d", count);
		sprintf(buff, "acc : %6.2f,%6.2f,%6.2f", imuLeft.acc[0], imuLeft.acc[1],
				imuLeft.acc[2]);
		LCD_DrawString(0, 40, buff);
		sprintf(buff, "%9.2f,%9.2f,%9.2f", imuLeft.gyro[0], imuLeft.gyro[1],
				imuLeft.gyro[2]);
		LCD_DrawString(0, 60, buff);
//		sprintf(buff, "%9.2f,%9.2f,%9.2f", imuLeft.gyro_offset[0], imuLeft.gyro_offset[1], imuLeft.gyro_offset[2]);
//		LCD_DrawString(0, 80, buff);
//		sprintf(buff, "mag : %6.0f,%6.0f,%6.0f", imuLeft.mag[0], imuLeft.mag[1], imuLeft.mag[2]);
//		LCD_DrawString(0, 100, buff);

//		imu_filter(imuLeft.acc[0], imuLeft.acc[1], imuLeft.acc[2], imuLeft.gyro[0], imuLeft.gyro[1], imuLeft.gyro[2]);
//		MahonyAHRSupdateIMU(imuLeft.gyro[0], imuLeft.gyro[1], imuLeft.gyro[2], imuLeft.acc[0], imuLeft.acc[1], imuLeft.acc[2]);
//		MahonyAHRSupdate(imuLeft.gyro[0], imuLeft.gyro[1], imuLeft.gyro[2],
//							imuLeft.acc[0], imuLeft.acc[1], imuLeft.acc[2],
//							imuLeft.mag[0], imuLeft.mag[1], imuLeft.mag[2]);
//		float roll, pitch, yaw;
//		eulerAngles(q_est, &roll, &pitch, &yaw);
		sprintf(buff, "ori : %6.2f,%6.2f,%6.2f", imuLeft.roll, imuLeft.pitch,
				imuLeft.yaw);
		LCD_DrawString(0, 120, buff);
		sprintf(buff, "acc: %6.2f", imuLeft.pitch_acc);
		LCD_DrawString(0, 160, buff);

//		for (int i = 0; i < 6; i++)
//			switch (states[i]) {
//			case IMU_IDLE:
//				LCD_DrawString(0, 180 + i * 16, "IDLE   ");
//				break;
//			case IMU_POS:
//				LCD_DrawString(0, 180 + i * 16, "POS    ");
//				break;
//			case IMU_NEG:
//				LCD_DrawString(0, 180 + i * 16, "NEG    ");
//				break;
//			case IMU_HIT_NP:
//				LCD_DrawString(0, 180 + i * 16, "HIT_NP ");
//				break;
//			case IMU_HIT_PN:
//				LCD_DrawString(0, 180 + i * 16, "HIT_PN ");
//				break;
//			case IMU_IDLE_NP:
//				LCD_DrawString(0, 180 + i * 16, "IDLE_NP");
//				break;
//			case IMU_IDLE_PN:
//				LCD_DrawString(0, 180 + i * 16, "IDLE_PN");
//				break;
//
//			};

//		sprintf(buff, "acc : %6.2f,%6.2f,%6.2f", imuRight.acc[0], imuRight.acc[1], imuRight.acc[2]);
//		LCD_DrawString(0, 120, buff);
//		sprintf(buff, "gyro: %6.2f,%6.2f,%6.2f", imuRight.gyro[0], imuRight.gyro[1], imuRight.gyro[2]);
//		LCD_DrawString(0, 140, buff);
//		sprintf(buff, "mag : %6.2f,%6.2f,%6.2f", imuRight.mag[0], imuRight.mag[1], imuRight.mag[2]);
//		LCD_DrawString(0, 160, buff);
//

		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_SET) {
			if (PC6 == BTN_UP) {
				PC6 = BTN_DOWN;
				initIMUStruct(&imuLeft);
			}
		} else {
			PC6 = BTN_UP;
		}

		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_SET) {
			if (PC7 == BTN_UP) {
				PC7 = BTN_DOWN;
				initIMUStruct(&imuRight);
			}
		} else {
			PC7 = BTN_UP;
		}
#endif

#ifdef TESTMUSIC

		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) {
			if (PA0 == BTN_UP) {
				PA0 = BTN_DOWN;
				// run command
				int res = setMusic(musicFilenames[musicNum]);
				playMusic();
//				seekMusic(0.5);
				musicNum = (musicNum + 1) % musicFileNum;
//				LCD_DrawString(0, 100, "PA0 down");

				WavHeader *header = &(sampleFile.header);
				sprintf(buff, "hertz: %d", header->sampleFreq);
				LCD_DrawString(30, 140, buff);
				sprintf(buff, "bits/sample: %d", header->bitsPerSample);
				LCD_DrawString(30, 160, buff);
				sprintf(buff, "channels: %d", header->channels);
				LCD_DrawString(30, 180, buff);
				sprintf(buff, "data: %d  ", header->dataChunkLength);
				LCD_DrawString(30, 200, buff);
				sprintf(buff, "debug %d %d %d     ", sampleFile.sampleCount,
						sampleFile.fileEmpty, sampleFile.inUse);
				LCD_DrawString(30, 220, buff);
			}
		} else {
			PA0 = BTN_UP;
//			LCD_DrawString(0, 100, "PA0 up  ");
		}

		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) {
			if (PC13 == BTN_UP) {
				PC13 = BTN_DOWN;
				// run command
				if (musicState == MUSIC_PLAYING) {
					pauseMusic();
				} else {
					playMusic();
				}
//				stopMusic();
//				seekMusic(0.5);
//				sprintf(buff, "prog: %.2f", getMusicProgress());
//				LCD_DrawString(0, 120, buff);
			}
		} else {
			PC13 = BTN_UP;
//			LCD_DrawString(0, 120, "PC13 up  ");
		}

//		sprintf(buff, "%d", numActiveDrums);
//		LCD_DrawString(200, 80, buff);
		drumUpdate();
		musicUpdate();
#endif

		updateButtons();
		InterfaceHandler();
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	__disable_irq();
	if (htim->Instance == TIM2) {
#ifdef TESTMUSIC
		precomputeMix();
#endif
	} else if (htim->Instance == TIM3) {
#ifdef USEIMU
		updateIMUs();
#endif
	}
	__enable_irq();
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
