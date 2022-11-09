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
//#define TESTFILE
void error_handler(int res, const char* msg) {
	char buff[100];
		sprintf(buff, "%d %s", res, msg);
	  LCD_DrawString(40, 0, buff);
	  HAL_Delay(1000000);
}
void display_success(int num, const char* msg) {
	char buff[100];
		sprintf(buff, "%d %s", num, msg);
	  LCD_DrawString(40, 0, buff);
//	  HAL_Delay(1000000);
}

char buff[31];
int counter = 1;
FIL emptyFile;
extern int numActiveDrums;
// stolen code
#define bit_set(var,bitno) ((var) |= 1 << (bitno))
#define bit_clr(var,bitno) ((var) &= ~(1 << (bitno)))
#define testbit(var,bitno) (((var)>>(bitno)) & 0x01)

imuStruct imuLeft = (imuStruct) {
	.port = GPIOE,
	.pin = GPIO_PIN_5
};
imuStruct imuRight = (imuStruct) {
	.port = GPIOE,
	.pin = GPIO_PIN_6
};

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
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
  /* USER CODE BEGIN 2 */

  /**
   * List of things to do
   *
   * audioChannelInit()
   * f_mount, f_opendir
   * for (f_readdir)
   * 	drumMatch()
   * audioInit()
   *
   * setFileName(&sampleFile, filename);
   * res = openFile(&sampleFile);
   *
   * while (1) {
   * 	readFile(&sampleFile);
   * 	if (!sampleFile.inUse) {
   * 		openFile(&sampleFile);
   * 	}
   *
   * 	drumPlay(...);
   * 	drumUpdate();
   * }
   *
   * void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
   * 	if (htim->Instance == TIM2) {
   * 	precomputeMix();
   * 	}
   * }
   *
   * ----------------------
   *
   * set up:
   * TIM4
   * TIM2 as slave of TIM4
   * DAC1 using TIM4 as trigger, DMA half-word circular
   * (DAC2)
   * FATFS, User Defined
   * - MAX_SS
   * - _F_LOCK tune up to 10 (maximum number of simultaneously opened files)
   * - change ff.h
   * SPI2 (Master Duplex)
   * - drag fatfs_sd.c
   * - drag fatfs_sd.h
   *
   * audio.c
   * audio.h
   * fileBuffer.h
   *
   * main.c
   * #include "fileBuffer.h"
   * #include "audio.h"
   */
  LCD_INIT();
  imu_setActive(&imuLeft);
  MPU9250_Init();
  MPU9250_SetAccelRange(ACCEL_RANGE_8G);
  MPU9250_SetGyroRange(GYRO_RANGE_1000DPS);
  imu_setActive(&imuRight);
  MPU9250_Init();
  MPU9250_SetAccelRange(ACCEL_RANGE_8G);
  MPU9250_SetGyroRange(GYRO_RANGE_1000DPS);
#ifdef TESTFILE
  audioChannelInit();
#endif

  FRESULT res;
  FATFS FatFs;
  DIR dir;
  FILINFO fno;

#ifdef TESTFILE
  // MOUNT THE SD CARD
  HAL_Delay(20);
  res = f_mount(&FatFs, "", 1);
  if (res != FR_OK) error_handler(res, "No sd card found!");
  HAL_Delay(20);

  res = f_opendir(&dir, "0:");
  if (res != FR_OK) error_handler(res, "Cannot open drive");
  HAL_Delay(20);

  // LIST ALL OF THE AVAILABLE FILES
  int lineNum = 0;
  char filename[15];
  while (1) {
	  res = f_readdir(&dir, &fno);
	  if (res != FR_OK || fno.fname[0] == 0) {
		  sprintf(buff, "%d no more :(", res);
		  LCD_DrawString(0, 20+20*(lineNum), buff);
		  break;
	  }
	  LCD_DrawString(0, 20+20*(lineNum++), fno.fname);
	  if (fno.fname[0] == 'M') strcpy(filename, fno.fname);
	  drumMatch(&fno);
  }

  // Initialize the fileStruct struct
  setFileName(&sampleFile, filename);
  res = openFile(&sampleFile);
  if (res != FR_OK) error_handler(res, "Cannot open file");
  // AUDIO INIT

  sprintf(buff, "audioINIT: %d", audioInit());
  LCD_DrawString(0, 0, buff);

  // Display some information about the header
  	WavHeader *header = &(sampleFile.header);
//    sprintf(buff, "hertz: %d", header->sampleFreq);
//    LCD_DrawString(30, 100, buff);
//    sprintf(buff, "bits/sample: %d", header->bitsPerSample);
//    LCD_DrawString(30, 120, buff);
//    sprintf(buff, "channels: %d", header->channels);
//    LCD_DrawString(30, 140, buff);
//    sprintf(buff, "data: %d", header->dataChunkLength);
//    LCD_DrawString(30, 160, buff);

#endif


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    int some_tick = HAL_GetTick();
    int pinC8 = 0;
    int pinC9 = 0;
#ifdef TESTFILE
//    HAL_DAC_Start_DMA(&hdac, audioLeft.channel, (uint32_t*)audioLeft.out, AUDIO_BUFFSIZE, DAC_ALIGN_12B_L);
#endif

	  imu_calibrateGyro(&imuLeft);
	//  imu_calibrateGyro(&imuRight);
  while (1)
  {

		if (HAL_GetTick() - some_tick > 200) {
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
			some_tick = HAL_GetTick();
		}

#ifndef TESTFILE

		imu_setActive(&imuLeft);
		MPU9250_GetData(imuLeft.raw, imuLeft.raw + 6, imuLeft.raw + 3); // raw is acc, gyro, mag
		// check here for the scale factors: https://github.com/MarkSherstan/STM32-MPU6050-MPU9250-I2C-SPI/blob/main/C/SPI/Core/Src/MPU9250.c
		for (int i = 0; i < 3; i++) {
			imuLeft.acc[i] = imuLeft.raw[i] / 2048.0;
			imuLeft.gyro[i] = (imuLeft.raw[i + 3] - imuLeft.gyro_offset[i]) / 32.8;
			imuLeft.mag[i] = imuLeft.raw[i + 6];
		}
//		imu_setActive(&imuRight);
//		MPU9250_GetData(imuRight.raw, imuRight.raw + 6, imuRight.raw + 3);
//		for (int i = 0; i < 3; i++) {
//			imuRight.acc[i] = 0.00239509262 * imuLeft.raw[i]; // 8 * 9.81 / 0x7FFF
//			imuRight.gyro[i] = 0.00026632423 * imuLeft.raw[i + 3] ; //  250 / 180 * 2 * 3.1415926 / 0x7FFF
//			imuRight.mag[i] = imuLeft.raw[i + 6];
//		}

		sprintf(buff, "acc : %6.2f,%6.2f,%6.2f", imuLeft.acc[0], imuLeft.acc[1], imuLeft.acc[2]);
		LCD_DrawString(0, 40, buff);
		sprintf(buff, "gyro: %6.2f,%6.2f,%6.2f", imuLeft.gyro[0], imuLeft.gyro[1], imuLeft.gyro[2]);
		LCD_DrawString(0, 60, buff);
		sprintf(buff, "offs: %6d,%6d,%6d", imuLeft.gyro_offset[0], imuLeft.gyro_offset[1], imuLeft.gyro_offset[2]);
		LCD_DrawString(0, 80, buff);
		sprintf(buff, "mag : %6.0f,%6.0f,%6.0f", imuLeft.mag[0], imuLeft.mag[1], imuLeft.mag[2]);
		LCD_DrawString(0, 100, buff);

//		sprintf(buff, "acc : %6.2f,%6.2f,%6.2f", imuRight.acc[0], imuRight.acc[1], imuRight.acc[2]);
//		LCD_DrawString(0, 120, buff);
//		sprintf(buff, "gyro: %6.2f,%6.2f,%6.2f", imuRight.gyro[0], imuRight.gyro[1], imuRight.gyro[2]);
//		LCD_DrawString(0, 140, buff);
//		sprintf(buff, "mag : %6.2f,%6.2f,%6.2f", imuRight.mag[0], imuRight.mag[1], imuRight.mag[2]);
//		LCD_DrawString(0, 160, buff);

#endif


#ifdef TESTFILE
		readFile(&sampleFile);
		if (!sampleFile.inUse) {
			openFile(&sampleFile);
		}

		sprintf(buff, "%d", numActiveDrums);
		LCD_DrawString(200, 80, buff);
		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == GPIO_PIN_RESET) {
			drumPlay(KICK);
			LCD_DrawString(220, 80, "8");
		}
		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET) {
			drumPlay(CRASH);
			LCD_DrawString(220, 80, "9");
		}
		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_10) == GPIO_PIN_RESET) {
			drumPlay(LOW_TOM);
			LCD_DrawString(220, 80, "0");
		}
		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_11) == GPIO_PIN_RESET) {
			drumPlay(HIGH_TOM);
			LCD_DrawString(220, 80, "1");
		}
		drumUpdate();
#endif
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		precomputeMix();
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
