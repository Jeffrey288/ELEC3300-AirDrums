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
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "rina.h"
#include "wav.h"
#include "fileBuff.h"
#include "audio.h"
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
DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac_ch1;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_FSMC_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM2_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define TESTFILE
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


//#define AUDIO_PRECOMP 	1
#define AUDIO_PRECOMP 	40
#define AUDIO_FREQ		22050
//#define AUDIO_FREQ		44100
#define SYSCLK_FREQ		72000000
#define AUDIO_BLOCKS	5
const uint16_t AUDIO_BUFFSIZE = (AUDIO_PRECOMP * AUDIO_BLOCKS);
//#define AUDIO_ARR		(SYSCLK_FREQ / AUDIO_FREQ - 1)
typedef union {
	struct {
		uint16_t left[AUDIO_PRECOMP];
		uint16_t right[AUDIO_PRECOMP];
	};
	uint16_t data[2][AUDIO_PRECOMP];
} DACBuff;
DACBuff dac_buff;

typedef struct {
	uint16_t out[AUDIO_PRECOMP * AUDIO_BLOCKS];
	uint16_t toWrite;
	uint16_t *curr;
	uint16_t *first;
	uint32_t channel;
	uint8_t onFlag;
} AudioChannel;
AudioChannel audioLeft;

char buff[31];
uint8_t fileBuffer[1000];
//uint16_t dac_out = 0;
int counter = 1;
uint16_t dac_out;
int16_t sample_sum;
FIL emptyFile;

// stolen code
#define bit_set(var,bitno) ((var) |= 1 << (bitno))
#define bit_clr(var,bitno) ((var) &= ~(1 << (bitno)))
#define testbit(var,bitno) (((var)>>(bitno)) & 0x01)
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
  /* USER CODE BEGIN 2 */
  LCD_INIT();
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

//  TIM2->PSC = 7900;
//  TIM2->ARR = 7900;
//  TIM2->ARR = 0;
  TIM4->ARR = SYSCLK_FREQ / AUDIO_FREQ - 1;
// TIM3->ARR = 72000000 / 22050 - 1;
//  TIM2->ARR = 72000000 / 23050 - 1;
 TIM2->PSC = 0;
 TIM2->ARR = AUDIO_PRECOMP - 1;
//  TIM2->ARR = 72000000 / 44100 - 1;

 for (int i = 0; i < AUDIO_BUFFSIZE; i++) {
	 audioLeft.out[i] = 0;
 }
 audioLeft.toWrite = 0;
 audioLeft.curr = audioLeft.first = audioLeft.out;
 audioLeft.onFlag = 0;
 audioLeft.channel = DAC_CHANNEL_1;

  FRESULT res;
  FATFS FatFs;
  DIR dir;
  FILINFO fno;

#ifdef TESTFILE
  HAL_Delay(20);
  res = f_mount(&FatFs, "", 1);
  if (res != FR_OK) error_handler(res, "No sd card found!");
  HAL_Delay(20);

  res = f_opendir(&dir, "0:");
  if (res != FR_OK) error_handler(res, "Cannot open drive");
  HAL_Delay(20);

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
  sprintf(buff, "audioINIT: %d", audioInit());
  LCD_DrawString(0, 0, buff);

  // Initialize the fileStruct struct
  setFileName(&sampleFile, filename);
  res = openFile(&sampleFile);
  if (res != FR_OK) error_handler(res, "Cannot open file");

  HAL_TIM_Base_Start(&htim4);
  HAL_TIM_Base_Start(&htim2);
  __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
//  __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);

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

  while (1)
  {

	  if (HAL_GetTick() - some_tick > 200) {
		  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
		  some_tick = HAL_GetTick();
	  }

//	  	  sprintf(buff, "(0) %4d (1) %4d", sampleFile.structs[0].buffSize, sampleFile.structs[1].buffSize);
//	  	  LCD_DrawString(0, 210, buff);
//	  	  sprintf(buff, "r:%d,w:%d,empty:%d,use:%d,%d,%d,%d", sampleFile.currReading, sampleFile.currWriting, sampleFile.fileEmpty, sampleFile.inUse,
//	  			(!fileStructEmpty(&sampleFile, sampleFile.currReading)),
//	  			(!fileStructEmpty(&sampleFile, (sampleFile.currReading + 1) % BUFF_NUM)),
//	  					0);
//	  	  LCD_DrawString(0, 230, buff);
//		  sprintf(buff, "sample: %6d     ", sample_sum);
//	      LCD_DrawString(20, 80, buff);

//		  if (sampleFile.sampleCount % 3241 == 0) {
////			  sprintf(buff, "count: %6d     ", sampleFile.sampleCount);
////			  LCD_DrawString(20, 80, buff);
//		  }

		readFile(&sampleFile);
		if (!sampleFile.inUse) {
		  openFile(&sampleFile);
		}

		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == GPIO_PIN_RESET) {
			if (!pinC8) {
				pinC8 = 1;
				drumPlay(KICK);
			}
		} else {
			pinC8 = 0;
		}
		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET) {
			if (!pinC9) {
				pinC9 = 1;
				drumPlay(CRASH);
			}
		} else {
			pinC9 = 0;
		}
		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_10) == GPIO_PIN_RESET) {
			drumPlay(LOW_TOM);
		}
		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_11) == GPIO_PIN_RESET) {
			drumPlay(HIGH_TOM);
		}
		drumUpdate();



		continue;

	  /**
	   * TIM2 CH3
	   * 72MHz -> 44.1kHz
	   * 72Mhz/44.1kHz = 1632.65306
	   * set AAR = 1632
	   *
	   * 72MHz->16kHz
	   * ARR = 4499
	   *
	   *
	   */


  //	  LCD_Clear(0, 0, 240, 320, BACKGROUND);
//  	  if (HAL_GetTick() - last_tick > 1000) {
//  		  last_tick = HAL_GetTick();
//  		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11));
//  	  }

//	  if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) {
//		  if (!btn_k2_pressed) {
//			  btn_k2_pressed = 1;
//
//		  }
//	  } else {
//		  btn_k2_pressed = 0;
//	  }


//
//      i++;

//      DAC_DHR12R1 = DAC_OUT[i++];
//      DAC->DHR12R1 = DAC_OUT[i++];
#ifdef TESTFILE

//			res = f_read(&fil, fileBuffer, 1000, &bytes_read);
//
//	  	if (res != FR_OK || bytes_read == 0) {
//
//	  	  f_close(&fil);
//
//	  	  res = f_open(&fil, filename, FA_READ);
//	  	  if (res != FR_OK) error_handler(res, "Cannot open file");
//
//	  	  res = f_read(&fil, &header, sizeof(WAV_HEADER), &bytes_read);
//	  	  if (res != FR_OK) error_handler(res, "Cannot read file");
//
//	  	  i=0;
//	  	  continue;
//	  	}
//
//	  	uint8_t *pt = fileBuffer;
//		  if (i % 33222 == 0) {
//			  sprintf(buff, "sample: %10d", i);
//			  LCD_DrawString(0, 0, buff);
//		  }

//	  while(bytes_read >0 ) {
//
////		  sprintf(buff, "bytes_read: %3d", bytes_read);
////			  LCD_DrawString(50, 180, buff);
//		  while (testbit(TIM2->SR, 0) == 0); /* wait for timer reset */
//		  bit_clr(TIM2->SR, 0);
//		  i++;
//		  bytes_read-=2;
//		  pt+=2;
////		  if (i % 2 == 0) continue;
//			sample_sum = (pt[1] << 8) | pt[0];
////			if (sample_sum < -32768)
////			sample_sum = -32768;
////			else if (sample_sum > 32767)
////			sample_sum = 32767;
//			sample_sum += 32768;
//			dac_out = sample_sum;
////			uint16_t output = sample_sum;
////			sprintf(buff, "data: %d", sample_sum);
////			LCD_DrawString(30, 200, buff);
//		  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (dac_out) >> 6);
////			sprintf(buff, "your mother %6d", sample_sum);
////			LCD_DrawString(30, 210, buff);
////			HAL_Delay(1000);
//
//	  }
#else
//	  if (i % 34331 == 0) {
//		  sprintf(buff, "sample: %10d", i);
//		  LCD_DrawString(0, 0, buff);
//	  }
	  uint16_t dac_out;
	  sample_sum = crash[i];
//	  if (sample_sum < -32768)
//		sample_sum = -32768;
//		else if (sample_sum > 32767)
//		sample_sum = 32767;
		sample_sum += 32768;
		dac_out = sample_sum;
		  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (dac_out) >> 4);
		  i = (i + 1) % MUSIC_LENGTH;
		while (testbit(TIM2->SR, 0) == 0); /* wait for timer reset */
		bit_clr(TIM2->SR, 0);
#endif
//	  	i++;
//	  	while (bytes_read > 0) {
//
//	//		sample_sum += crash[i];
//	//
//
//		  i++;
//		  bytes_read -= 2;
//		  pt += 2;
//
//	  	}

//
//      i = (i + 1) % MUSIC_LENGTH;

//      while (testbit(TIM2->SR, 0) == 0); /* wait for timer reset */
//      bit_clr(TIM2->SR, 0);
//      HAL_Delay(1000);

  //	  now_LR = 100;
  //	  LCD_DrawEllipse(100, 200, prev_SR, prev_LR, BACKGROUND);
  //	  LCD_Clear(120 - prev_LR, 200 - prev_SR, prev_LR * 2, prev_SR *2, BACKGROUND);

  //	  HAL_Delay(10);
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

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_T4_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT2 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_T2_TRGO;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */
  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_GATED;
  sSlaveConfig.InputTrigger = TIM_TS_ITR3;
  if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|SD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 SD_CS_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PD12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PC8 PC9 PC10 PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

}

/* FSMC initialization function */
static void MX_FSMC_Init(void)
{

  /* USER CODE BEGIN FSMC_Init 0 */

  /* USER CODE END FSMC_Init 0 */

  FSMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FSMC_Init 1 */

  /* USER CODE END FSMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /** Disconnect NADV
  */

  __HAL_AFIO_FSMCNADV_DISCONNECTED();

  /* USER CODE BEGIN FSMC_Init 2 */

  /* USER CODE END FSMC_Init 2 */
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
//		counter++;
//		sample_sum = readSample(&sampleFile);
//		dac_buff.left[0] = (sample_sum + 32768);
//		dac_out = (sample_sum + 32768);
//		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (dac_out) >> 6);
//		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (dac_buff.left[0]) >> 6);
//		HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
		for (int i = 0; i < AUDIO_PRECOMP; i++) {
			sample_sum = 0;
			sample_sum += readSample(&sampleFile);
			sample_sum += drumMix();
			*(audioLeft.curr++) = (sample_sum + 32768);
//			dac_buff.left[i] >>= 4;
		}
		audioLeft.toWrite++;
		if (audioLeft.toWrite >= AUDIO_BLOCKS) {
			audioLeft.curr = audioLeft.first;
			audioLeft.toWrite = 0;
		}
//		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)dac_buff.left, AUDIO_PRECOMP, DAC_ALIGN_12B_L);
		if (!audioLeft.onFlag) {
			HAL_DAC_Start_DMA(&hdac, audioLeft.channel, (uint32_t*)audioLeft.out, AUDIO_BUFFSIZE, DAC_ALIGN_12B_L);
			audioLeft.onFlag = 1;
		}
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
