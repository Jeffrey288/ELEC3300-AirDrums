#ifndef __GUI_H
#define __GUI_H

#include <stdio.h>
#include "lcd.h"
#include "bsp_ili9341_lcd.h"
#include "bsp_xpt2046_lcd.h"

#include "palette.h"
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
#include "WavImage.h"

static int hey = 1;

static int position = 0;
static short stopstatus = 0;
static short recordingstatus = 0;
static int currentfile;

static char filenamearray[4][15] = { "SongA", "SongB", "SongC", "SongD" };
static int posinfo[2] = { 0, 0 };

static int GUISTACK[5] = { 0, 0, 0, 0, 0 };
static short PLAYPAUSESTATUS = 0;
static short starttimeline = 0;
static int MusicSpectrumArray[30] = { 0 };

static int RGB = 0x0000;
static short upper = 0x0010;
static short musicplayeron = 0;
static int status = 1;
static int timer = 0;
static short drumpractice[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 };
static short filereturn = 0;
static int currentinterface = -1;

// GUI Stack Functions

static void GUIFORWARD(int functionindex, int GUISTACK[5]) {
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

static void GUIBACKWARD(int GUISTACK[5]) {
	for (int i = 0; i < 4; i++) {
		if (GUISTACK[i] == 0)
			break;
		GUISTACK[i] = GUISTACK[i + 1];
	}

}

static short GUIEMPTYSTACK(int GUISTACK[5]) {
	return (GUISTACK[0] == -1);
}

// Specific GUI Functions


static void VolumeControlInterface() {
	static int volumecurrentstatus = -1; // will only init the variable once,
	static int volumeprevstatus = -1; // but the value of volumeprevstatus will
	// stay the same even after finish function

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	//uint32_t abc = HAL_ADC_GetValue(&hadc1);
	char xposition[4];
	sprintf(xposition, "%04d", HAL_ADC_GetValue(&hadc1));
	LCD_DrawString(200, 20, "ADC");
	LCD_DrawString(240, 20, xposition);

	volumecurrentstatus = VolumeStatus(HAL_ADC_GetValue(&hadc1), 4500);
	char yposition[1];
	sprintf(yposition, "%01d", volumecurrentstatus);
	LCD_DrawString(200, 40, "index");
	LCD_DrawString(240, 40, yposition);

	if (volumecurrentstatus != volumeprevstatus) {
		VolumeControl(240, 120, volumecurrentstatus);
		volumeprevstatus = volumecurrentstatus;
	}
}

static short boundarychecker(int inputx, int inputy, int lowlimitx, int highlimitx,
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

static void ReadSDCard(int numoffiles, char filename[][9], int filetype) {
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

static int FileSelector(int XPOS, int YPOS, int page, int numberoffiles) {

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
static void FileFunctionMenu(int index) {
	LCD_Clear(0, 0, 320, 240, WHITE);
	LCD_DrawString(20, 20, filenamearray[index]);
	//imagebuilder(10, 60,136,115, WAVLARGE);
	//imagebuilder(120, 60,57,197, DrumPlay);
	//imagebuilder(120, 120,57,198, SoloPlay);

	//imagebuilder(280, 190,32,32, backward);

	//LCD_DrawString(25+100*i,100+110*j,filename[counter] );
}

static void MusicTimeline(int xpos) {
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

static void MusicSpectrum(int newpulse) {

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
static void MusicPlayerControl(int xpos, int ypos) {
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

static int adcstimulate() {
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	return (HAL_ADC_GetValue(&hadc1));
}

static void MusicPlayerHandler(int xpos, int ypos, short mode) {
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

static void MusicPlayerInterface(short mode) {
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

static void printcurrentstack() {
	char xposition[1];
	sprintf(xposition, "%01d", GUISTACK[0]);
	LCD_DrawString(40, 20, "STACK");
	LCD_DrawString(90, 20, xposition);

}

static void MainMenuInterface() {
	imagebuilder(10, 20, 102, 101, MusicPlayer);
	LCD_DrawString(20, 120, "MusicPlayer");
	imagebuilder(115, 20, 99, 98, DrumPractice);
	LCD_DrawString(120, 120, "DrumPractice");
	imagebuilder(215, 20, 99, 98, Metronome);
	LCD_DrawString(230, 120, "Metronome");

}

static void printtouchposition(int xpos, int ypos) {
	char xposition[3];
	sprintf(xposition, "%03d", xpos);
	LCD_DrawString(40, 200, "XPOS");
	LCD_DrawString(90, 200, xposition);
	char yposition[3];
	sprintf(yposition, "%03d", ypos);
	LCD_DrawString(40, 220, "YPOS");
	LCD_DrawString(90, 220, yposition);

}

static void InterfaceSelector(int xpos, int ypos, int currentinterface) {
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
static void metronome(int BPM) {
//	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

static void DrumPratice() {
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

static void CurrentInterface(int currentinterface) {
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

#endif __GUI_H
