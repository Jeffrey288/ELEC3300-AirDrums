#if 0

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
#include <stdio.h>
#include "lcd.h"

#include "bsp_ili9341_lcd.h"
#include "bsp_xpt2046_lcd.h"
#include "palette.h"
#include "WAV.h"
//#include "WAVLARGE.h"
#include "backward.h"
#include "PlayButton.h"
#include "PauseButton.h"
#include "StopButton.h"
#include "MusicPlayer.h"
#include "DrumPractice.h"
#include "Metronome.h"
#include "Recording.h"
#include "Taiko.h"
#include "Return.h"

//#include "GroupPic.h"
//#include "DrumPlay.h"
//#include "SoloPlay.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim1;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FSMC_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

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
	MX_FSMC_Init();
	MX_ADC1_Init();
	MX_TIM1_Init();
	/* USER CODE BEGIN 2 */
	ILI9341_Init();
	ILI9341_GramScan(3);
	LCD_SetBackColor(WHITE);
	ILI9341_Clear(0, 0, 320, 240);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	int position = 0;
	int volumecurrentstatus = -1;
	int volumeprevstatus = -1;
	startlogo();
	while (StartProcess())
		; // Wait starting signal
	LCD_Clear(0, 0, 320, 240, WHITE);
	LCD_SetBackColor(WHITE);

	Interface(0, -1, -1);

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);

	short stopstatus = 0;
	short recordingstatus = 0;
	int currentfile;



	char filenamearray[4][9];
	strcpy(filenamearray[0], "SongA");
	strcpy(filenamearray[1], "SongB");
	strcpy(filenamearray[2], "SongC");
	strcpy(filenamearray[3], "SongD");
	int posinfo[2] = { 0, 0 };

	int GUISTACK[5] = { 0, 0, 0, 0, 0 };
	short PLAYPAUSESTATUS = 0;

	void GUIFORWARD(int functionindex, int GUISTACK[5]) {
		if (functionindex != GUISTACK[0]) {
			for (int i = 0; i < 5; i++) {
				if (GUISTACK[i] == 0) {
					for (int j = i; j > 0; j--) {
						GUISTACK[j] = GUISTACK[j - 1];
					}
					GUISTACK[0] = functionindex;
					break;
				}
			}
		}
	}

	void GUIBACKWARD(int GUISTACK[5]) {
		for (int i = 0; i < 4; i++) {
			if (GUISTACK[i] == 0)

				break;
			GUISTACK[i] = GUISTACK[i + 1];
		}

	}

	short GUIEMPTYSTACK(int GUISTACK[5]) {
		return (GUISTACK[0] == -1);
	}

	short boundarychecker(int inputx, int inputy, int lowlimitx, int highlimitx,
			int lowlimity, int highlimity) {
		if ((inputx > lowlimitx) && (inputx < highlimitx)) {
			if ((inputy > lowlimity) && (inputy < highlimity)) {

				//LCD_DrawString(40,20,"BoundaryTrue");
				return 1;
			}
		}
		//LCD_DrawString(40,20,"BoundaryFalse");
		return 0;

	}

	void ReadSDCard(int numoffiles, char filename[][9], int filetype) {
		//int rows =  1; //numoffiles %3;
		LCD_Clear(0, 0, 320, 240, WHITE);
		int counter = 0;
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 2; i++) {
				imagebuilder(20 + 100 * i, 20 + 110 * j, 72, 85, WAV);
				LCD_DrawString(25 + 100 * i, 110 + 110 * j, filename[counter]);
				counter++;
			}
		}
	}

	int FileSelector(int XPOS, int YPOS, int page, int numberoffiles) {

		if ((boundarychecker(XPOS, YPOS, 0, 120, 0, 120))
				&& ((page + 1) <= numberoffiles))
			return (page + 0);
		else if ((boundarychecker(XPOS, YPOS, 120, 240, 0, 120))
				&& ((page + 2) <= numberoffiles))
			return (page + 1);

		if ((boundarychecker(XPOS, YPOS, 0, 120, 120, 240))
				&& ((page + 3) <= numberoffiles))
			return (page + 2);
		else if ((boundarychecker(XPOS, YPOS, 120, 240, 120, 240))
				&& ((page + 4) <= numberoffiles))
			return (page + 3);

	}
	void FileFunctionMenu(int index) {
		LCD_Clear(0, 0, 320, 240, WHITE);
		LCD_DrawString(20, 20, filenamearray[index]);
		//imagebuilder(10, 60,136,115, WAVLARGE);
		//imagebuilder(120, 60,57,197, DrumPlay);
		//imagebuilder(120, 120,57,198, SoloPlay);

		//imagebuilder(280, 190,32,32, backward);

		//LCD_DrawString(25+100*i,100+110*j,filename[counter] );
	}
	short starttimeline = 0;
	void MusicTimeline(int xpos) {
		//Start:30
		//End: 300
		if ((xpos > 30) && (xpos < 300)) {
			if (starttimeline == 0) {
				starttimeline = 1;
				LCD_Clear(0, 190, 320, 40, WHITE);
				//Front
				if ((xpos - 40) > 0) {
					if (recordingstatus == 1) {
						LCD_SetTextColor(RED);
					} else {
						LCD_SetTextColor(BLACK);
					}
					ILI9341_DrawRectangle(30, 200, xpos - 40, 5, WHITE);
				}
				//Back
				if ((300 - xpos - 10) > 0) {
					LCD_SetTextColor(0xF700);
					ILI9341_DrawRectangle(xpos + 10, 200, 300 - xpos - 10, 5,
							WHITE);
				}

				LCD_SetTextColor(GREEN);
				ILI9341_DrawCircle(xpos, 200, 10, WHITE);

				HAL_Delay(100);
			} else {
				LCD_Clear(xpos - 18, 180, 10, 40, WHITE);
				if ((xpos - 40) > 0) {
					if (recordingstatus == 1) {
						LCD_SetTextColor(RED);
					} else {
						LCD_SetTextColor(BLACK);
					}
					ILI9341_DrawRectangle(xpos - 18, 200, 10, 5, WHITE);
				}
//			  if((300-xpos-10)>0)
//			  		  {
//			  		  LCD_SetTextColor(0xF700);
//			  		  ILI9341_DrawRectangle(xpos+10,200,300-xpos-10,5,WHITE);
//			  		  }
//
				LCD_SetTextColor(GREEN);
				ILI9341_DrawCircle(xpos, 200, 10, WHITE);

			}
		}

	}

	int MusicSpectrumArray[30] = { 0 };

	int RGB = 0x0000;
	short upper = 0x0010;
	void MusicSpectrum(int newpulse) {

		for (int i = 30; i > 0; i--) {
			MusicSpectrumArray[i] = MusicSpectrumArray[i - 1];
			MusicSpectrumArray[0] = newpulse;
		}
		if (RGB == 0xFFFF)
			RGB = 0X0000;
		LCD_SetTextColor(RGB);
		RGB += upper;

		LCD_Clear(10, 150, 310, 70, WHITE);
		for (int i = 0; i < 30; i++) {
			int increasefactor = MusicSpectrumArray[i] / 90;

			ILI9341_DrawRectangle(10 + (i * 10), 200 - increasefactor, 5,
					20 + increasefactor, WHITE);
			//ILI9341_DrawRectangle(310,200,5,20,WHITE);
		}

	}
	void MusicPlayerControl(int xpos, int ypos) {
		if ((PLAYPAUSESTATUS == 1)
				&& boundarychecker(xpos, ypos, 100, 160, 120, 170)) {
			LCD_Clear(100, 120, 60, 60, WHITE);
			imagebuilder(100, 120, 57, 57, PlayButton);
			PLAYPAUSESTATUS = 0;
			HAL_Delay(100);
		} else if ((PLAYPAUSESTATUS == 0)
				&& boundarychecker(xpos, ypos, 100, 160, 120, 170)) {
			LCD_Clear(100, 120, 60, 60, WHITE);
			imagebuilder(100, 120, 56, 57, PauseButton);
			PLAYPAUSESTATUS = 1;
			HAL_Delay(100);
		} else if (boundarychecker(xpos, ypos, 200, 260, 120, 170)) {

		}
	}
	int adcstimulate() {
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 1000);
		return (HAL_ADC_GetValue(&hadc1));
	}

	void MusicPlayerHandler(int xpos, int ypos, short mode) {
//	  while(!(XPT2046_TouchDetect() == TOUCH_PRESSED));
//	  XPT2046_Touch(posinfo);

		if ((PLAYPAUSESTATUS == 0)
				&& boundarychecker(xpos, ypos, 20, 80, 80, 140)) {
			LCD_Clear(20, 80, 60, 60, WHITE);
			imagebuilder(20, 80, 56, 57, PauseButton);
			PLAYPAUSESTATUS = 1;
			HAL_Delay(300);
		} else if ((PLAYPAUSESTATUS == 1)
				&& boundarychecker(xpos, ypos, 20, 80, 80, 140)) {
			LCD_Clear(20, 80, 60, 60, WHITE);
			imagebuilder(20, 80, 57, 57, PlayButton);
			PLAYPAUSESTATUS = 0;
			HAL_Delay(300);
		} else if (boundarychecker(xpos, ypos, 80, 140, 80, 140)) {
			stopstatus = 1;

		}

		else if (boundarychecker(xpos, ypos, 140, 200, 80, 140)) {
			if (recordingstatus == 1)
				recordingstatus = 0;
			else if (recordingstatus == 0)
				recordingstatus = 1;

			HAL_Delay(200);

		}

	}

	short musicplayeron = 0;
	int status = 1;

	void MusicPlayerInterface(short mode) {
		int timestepupcounter = 31;
		if (musicplayeron == 0) {
			imagebuilder(20, 80, 57, 57, PlayButton);
			imagebuilder(80, 80, 57, 56, StopButton);
			imagebuilder(140, 80, 56, 57, Recording);
			LCD_DrawString(25, 40, filenamearray[currentfile]);
			//imagebuilder(260, 210, 31, 28, Return);
			MusicTimeline(31);
			status = 1;
			starttimeline = 0;

		}

		while (status == 1) {
			if (XPT2046_TouchDetect() == TOUCH_PRESSED) {
				XPT2046_Touch(posinfo);
				if ((posinfo[0] > 260) && (posinfo[1] > 180)
						&& (!GUIEMPTYSTACK(GUISTACK))) {
					GUIBACKWARD(GUISTACK);
					status = 0;
					break;
				}
				MusicPlayerHandler(posinfo[0], posinfo[1], mode);
			}

			VolumeControlInterface();
			if (stopstatus == 1) {
				LCD_Clear(20, 80, 60, 60, WHITE);
				imagebuilder(20, 80, 57, 57, PlayButton);
				PLAYPAUSESTATUS = 0;
				starttimeline = 0;
				timestepupcounter = 31;
				recordingstatus = 0;
				stopstatus = 0;
				MusicTimeline(31);

			}
			if (PLAYPAUSESTATUS == 1) {
				MusicTimeline(timestepupcounter / 10);
				timestepupcounter++;
			}

		}

		//MusicPlayerControl(posinfo[]);

	}

	//ReadSDCard(6,filenamearray ,1);

	void printcurrentstack() {
		char xposition[1];
		sprintf(xposition, "%01d", GUISTACK[0]);
		LCD_DrawString(40, 20, "STACK");
		LCD_DrawString(90, 20, xposition);

	}

	void MainMenuInterface() {
		imagebuilder(10, 20, 102, 101, MusicPlayer);
		LCD_DrawString(20, 120, "MusicPlayer");
		imagebuilder(115, 20, 99, 98, DrumPractice);
		LCD_DrawString(120, 120, "DrumPractice");
		imagebuilder(215, 20, 99, 98, Metronome);
		LCD_DrawString(230, 120, "Metronome");

	}

	void printtouchposition(int xpos, int ypos) {
		char xposition[3];
		sprintf(xposition, "%03d", xpos);
		LCD_DrawString(40, 200, "XPOS");
		LCD_DrawString(90, 200, xposition);
		char yposition[3];
		sprintf(yposition, "%03d", ypos);
		LCD_DrawString(40, 220, "YPOS");
		LCD_DrawString(90, 220, yposition);

	}

	//MusicTimeline(30);

	int timer = 0;
	//MusicSpectrum(30);
	MainMenuInterface();

	//MusicPlayerInterface(30);

	void InterfaceSelector(int xpos, int ypos, int currentinterface) {
		if ((posinfo[0] > 260) && (posinfo[1] > 180)
				&& (!GUIEMPTYSTACK(GUISTACK))) {
			GUIBACKWARD(GUISTACK);
		}

		else {
			if (currentinterface == 0) {
				printtouchposition(0, 0);
				if (boundarychecker(xpos, ypos, 0, 110, 0, 120)) // Mainmenu --> MusicPlayer
						{
					GUIFORWARD(1, GUISTACK);
				} else if (boundarychecker(xpos, ypos, 115, 215, 0, 120)) {
					GUIFORWARD(5, GUISTACK);

				} else if (boundarychecker(xpos, ypos, 215, 315, 0, 120)) {
					GUIFORWARD(8, GUISTACK);
				}

			} else if (currentinterface == 1) {
				musicplayeron = 0;
				GUIFORWARD(3, GUISTACK);

			}
//		  else if (currentinterface ==2)
//		  {
//			  if (boundarychecker(xpos,ypos,120,320,60,119))
//			  {
//				  GUIFORWARD(3,GUISTACK);
//			  }
//			  else if (boundarychecker(xpos,ypos,120,320,121,180))
//			  {
//				  GUIFORWARD(4,GUISTACK);
//			  }
//		  }
			else if (currentinterface == 3) {
				musicplayeron = 1;
				while ((XPT2046_TouchDetect() != TOUCH_PRESSED)) {
					MusicPlayerInterface(0);
				}

			}

		}

	}
	void metronome(int BPM) {
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

	}

	short drumpractice[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
			0, 0, 0 };

	void DrumPratice() {
		imagebuilder(10, 180, 45, 46, Taiko);
		int stepcounter = 0;
		while (1) {
			if ((XPT2046_TouchDetect() == TOUCH_PRESSED)) {
				XPT2046_Touch(posinfo);
				if (boundarychecker(posinfo[0], posinfo[1], 200, 325, 120, 240))
					break;
			}
			if (stepcounter == 30) {
				stepcounter = 0;
			}
			LCD_Clear(60, 120, 260, 120, WHITE);
			for (int i = 2000; i > 0; i--) {
				if ((i % 100) == 0) {
					LCD_Clear(60 + (i * 20), 120, 10, 10, WHITE);
					LCD_SetTextColor(GREEN);
					ILI9341_DrawCircle(60 + (i * 20), 210, 10, WHITE);
				}

			}

			stepcounter++;
		}

	}

	short filereturn = 0;
	void CurrentInterface(int currentinterface) {
		switch (currentinterface) {
		case 0: //MainMenu
			LCD_Clear(0, 0, 320, 240, WHITE);
			MainMenuInterface();
			break;
		case 1: // MusicPlayer
			LCD_Clear(0, 0, 320, 240, WHITE);

			ReadSDCard(6, filenamearray, 1);
			if (filereturn == 1) {
				filereturn = 0;
				currentfile = -1;
			}
			imagebuilder(260, 190, 31, 28, Return);
			break;
		case 2: //FileFunctionMenu
//	  		  LCD_Clear(0,0,320,240,WHITE);
//	  		  if (filereturn ==0)
//	  		  {
//	  			  currentfile = FileSelector(posinfo[0],posinfo[1],0,4);
//	  		  }
//
//	  	  	  FileFunctionMenu(currentfile);
			break;
		case 3: // MusicPlayer-DrumPlay
			currentfile = FileSelector(posinfo[0], posinfo[1], 0, 4);
			LCD_Clear(0, 0, 320, 240, WHITE);
			filereturn = 1;
			stopstatus = 0;
			recordingstatus = 0;
			MusicPlayerInterface(0);
//	  		  imagebuilder(260, 2100, 31, 28, Return);
			break;
		case 4: // MusicPlayer-SoloPlay
//	  		  LCD_Clear(0,0,320,240,WHITE);
//	  		  filereturn =1;
			break;
		case 5:
			LCD_Clear(0, 0, 320, 240, WHITE);
			DrumPratice();

		case 8:
			LCD_Clear(0, 0, 320, 240, WHITE);
			imagebuilder(260, 210, 31, 28, Return);
			metronome(10);
			break;

		}

	}

	int currentinterface = -1;
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		//VolumeControlInterface();
//	  MusicTimeline(30 + timer);
//	  	  timer++;
//	  	  HAL_Delay(100);
//	  HAL_ADC_Start(&hadc1);
//	  HAL_ADC_PollForConversion(&hadc1, 1000);
//
//	  MusicSpectrum(HAL_ADC_GetValue(&hadc1));
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

		if (XPT2046_TouchDetect() == TOUCH_PRESSED) {
			XPT2046_Touch(posinfo);
			InterfaceSelector(posinfo[0], posinfo[1], GUISTACK[0]);
			if (currentinterface != GUISTACK[0]) {
				CurrentInterface(GUISTACK[0]);
				currentinterface = GUISTACK[0];
			}

			//printcurrentstack();
			//printtouchposition(posinfo[0],posinfo[1]);
			//MusicPlayerControl(posinfo[0],posinfo[1]);
			//MusicPlayerInterface(posinfo[0]);

		}

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

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Common config
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_15;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 0;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 63;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2 | GPIO_PIN_1, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_5,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12 | GPIO_PIN_13, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);

	/*Configure GPIO pins : PE2 PE0 PE1 */
	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : PE3 PE4 */
	GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 PB1 PB5 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PD12 PD13 */
	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* FSMC initialization function */
static void MX_FSMC_Init(void) {

	/* USER CODE BEGIN FSMC_Init 0 */

	/* USER CODE END FSMC_Init 0 */

	FSMC_NORSRAM_TimingTypeDef Timing = { 0 };
	FSMC_NORSRAM_TimingTypeDef ExtTiming = { 0 };

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
	hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
	hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
	hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
	/* Timing */
	Timing.AddressSetupTime = 0;
	Timing.AddressHoldTime = 15;
	Timing.DataSetupTime = 26;
	Timing.BusTurnAroundDuration = 0;
	Timing.CLKDivision = 16;
	Timing.DataLatency = 17;
	Timing.AccessMode = FSMC_ACCESS_MODE_A;
	/* ExtTiming */
	ExtTiming.AddressSetupTime = 0;
	ExtTiming.AddressHoldTime = 15;
	ExtTiming.DataSetupTime = 1;
	ExtTiming.BusTurnAroundDuration = 0;
	ExtTiming.CLKDivision = 16;
	ExtTiming.DataLatency = 17;
	ExtTiming.AccessMode = FSMC_ACCESS_MODE_A;

	if (HAL_SRAM_Init(&hsram1, &Timing, &ExtTiming) != HAL_OK) {
		Error_Handler();
	}

	/** Disconnect NADV
	 */

	__HAL_AFIO_FSMCNADV_DISCONNECTED();

	/* USER CODE BEGIN FSMC_Init 2 */

	/* USER CODE END FSMC_Init 2 */
}

/* USER CODE BEGIN 4 */

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

#endif
