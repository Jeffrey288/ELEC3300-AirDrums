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

#include "audio.h"

typedef enum {
	GUI_MainMenu = 0,
	GUI_SongSelection = 1,
	GUI_SongPlayer = 3,
	GUI_DrumPractice = 5,
	GUI_Metronome = 8,
} GUIPage;

// GUISTACKS
// currentinterface specifys what gui is being displayed now
static int currentinterface = -1;
static int GUISTACK[5] = { 0, 0, 0, 0, 0 };

// stores the file being selected
static int currentfile = -1;

// stores whether recording is on
typedef enum {
	RecordingOff, RecordingOn,
} RecordingState;

static RecordingState recordingstatus = RecordingOff;

static int position = 0;
static short stopstatus = 0;
static char filenamearray[4][9] = { "SongA", "SongB", "SongC", "SongD" };
static int posinfo[2] = { 0, 0 };

static short PLAYPAUSESTATUS = 0;
static short starttimeline = 0;
static int MusicSpectrumArray[30] = { 0 };

static int RGB = 0x0000;
static short upper = 0x0010;
static short musicplayeron = 0;
static int status = 1;
static int timer = 0;
static short drumpractice[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		0, 0 };
static short filereturn = 0;

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

// For debugging the GUI stack

static void printcurrentstack() {
	char xposition[1];
	sprintf(xposition, "%01d", GUISTACK[0]);
	LCD_DrawString(40, 20, "STACK");
	LCD_DrawString(90, 20, xposition);
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

// Touchscreen helper funciton

static inline short boundarychecker(int inputx, int inputy, int lowlimitx,
		int highlimitx, int lowlimity, int highlimity) {
	return (inputx > lowlimitx) && (inputx < highlimitx) && (inputy > lowlimity) && (inputy < highlimity);
//	if ((inputx > lowlimitx) && (inputx < highlimitx)) {
//		if ((inputy > lowlimity) && (inputy < highlimity)) {
//			//LCD_DrawString(40,20,"BoundaryTrue");
//			return 1;
//		}
//	}
//	//LCD_DrawString(40,20,"BoundaryFalse");
//	return 0;

}

/**
 * Specific GUI Functions
 */

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

static void SongSelectionInterface(int numoffiles, char filename[][15],
		int filetype) {
	//int rows =  1; //numoffiles %3;
	LCD_Clear(0, 0, 320, 240, WHITE);
	int counter = 0;
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 2; i++) {
			imagebuilder(20 + 100 * i, 20 + 110 * j, 72, 85, WAV);
//			char tempbuff[9];
//			memcpy(tempbuff, filename[counter], 8);
			LCD_DrawString(25 + 100 * i, 110 + 110 * j,
					musicFilenames[counter]);
			counter++;
		}
	}
}

// Touch screen handler for choosing the file
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

#define LCD_DrawCircle(x, y, rad, color) \
	LCD_SetTextColor(color); \
	ILI9341_DrawCircle(x, y, rad, 1);
//	LCD_DrawEllipse((x) - (rad), (y) - (rad), rad, rad, color);
#define LCD_DrawRectangle(x, y, w, h, color) \
	LCD_Clear(x, y, w, h, color);
uint16_t max(uint16_t a, uint16_t b) {return (a > b) ? a : b;}

static int prev_xpos = 0;
static void MusicTimeline(float pos, uint8_t drawWhole) {
	// pos: position of the current cursor
	// drawWhole: 1 if redraw the whole timeline, 0 if not

	// Start: 30, End: 300
	uint16_t xpos = 30 + (300 - 30) * pos;
//	char buff[20];
//	sprintf(buff, "diu %3d", xpos);
//	LCD_DrawString(0, 0, buff);

	if (drawWhole) {

		// Draw the playbar
		LCD_DrawRectangle(30, 200, max((xpos - 30) - 10, 0), 5, (recordingstatus == 1) ? RED : BLACK); // BEFORE THE THING
		LCD_DrawRectangle(xpos + 10, 200, max((300 - xpos) - 10, 0) , 5, 0xF700); // AFTER THE THING

		// Draw the bob
		LCD_DrawCircle(xpos, 200, 10, GREEN);

	} else {

		// Draw the playbar
		if (prev_xpos < xpos) {
			LCD_DrawRectangle(prev_xpos - 10, 190, (xpos - prev_xpos), 40, WHITE);
			LCD_DrawRectangle(prev_xpos - 10, 200, (xpos - prev_xpos), 5, (recordingstatus == 1) ? RED : BLACK);
		} else {
			LCD_DrawRectangle(xpos + 10, 190, (prev_xpos - xpos + 1), 40, WHITE);
			LCD_DrawRectangle(xpos + 10, 200, (prev_xpos - xpos + 1), 5, 0xF700);
		}

		// Draw the bob
		LCD_DrawCircle(xpos, 200, 10, GREEN);

	}

	prev_xpos = xpos;

}

static void MusicPlayerInterfaceSelector(int xpos, int ypos, short mode) {
//	char buff[20];
//	sprintf(buff, "diu %3d %3d", xpos, ypos);
//	LCD_DrawString(100, 20, buff);
//	sprintf(buff, "diu %.3f", getMusicProgress());
//	LCD_DrawString(100, 60, buff);
	if (boundarychecker(xpos, ypos, 20, 80, 80, 140)) { // if the play/paused button is pressed
		switch (musicState) {
		case MUSIC_UNINITED: // when the song is paused
		case MUSIC_PAUSED: // clicking the play button will load/play the song

			// load and play the song
			if (musicState == MUSIC_UNINITED)
				setMusic(musicFilenames[currentfile]);
			playMusic(); // this will automatically update the state to MUSIC_PLAYING

			// change the button to the play button
			LCD_Clear(20, 80, 60, 60, WHITE);
			imagebuilder(20, 80, 56, 57, PauseButton);

			break;
		case MUSIC_PLAYING: // when the song is playing
			// and the pause button is pressed

			// the music is paused
			pauseMusic();

			LCD_Clear(20, 80, 60, 60, WHITE);
			imagebuilder(20, 80, 57, 57, PlayButton); // draw the play button

			break;
		}
	} else if (boundarychecker(xpos, ypos, 80, 140, 80, 140)) { // if the stop button is pressed
		// stop the music, unload the file
		stopMusic(); // this will automatically update the musicState to MUSIC_UNINITED

		// change the pause button to a play button
		LCD_Clear(20, 80, 60, 60, WHITE);
		imagebuilder(20, 80, 57, 57, PlayButton); // draw the play button
	} else if (boundarychecker(xpos, ypos, 140, 200, 80, 140)) { // if the recording button is pressed
		// to be implemented
		if (recordingstatus == RecordingOn) recordingstatus = RecordingOff;
		else /* recordingstatus == RecordingOff */recordingstatus = RecordingOn;
	} else if (boundarychecker(xpos, ypos, 30, 300, 180, 200)) {// Music Drag Timeline
		float RelativeMusicPosition = (xpos - 30) / 270.0;
//		sprintf(buff, "diu %.3f", RelativeMusicPosition);
//		LCD_DrawString(100, 40, buff);
		seekMusic(RelativeMusicPosition);
		musicUpdate();
	}

}

static void MusicPlayerInterface(short mode) { // Draws the interface for the music player
	imagebuilder(20, 80, 57, 57, PlayButton);
	imagebuilder(80, 80, 57, 56, StopButton);
	imagebuilder(140, 80, 56, 57, Recording);
	LCD_DrawString(25, 40, musicFilenames[currentfile]);
	MusicTimeline(0, 1);
}

static void MainMenuInterface() {
	imagebuilder(10, 20, 102, 101, MusicPlayer);
	LCD_DrawString(20, 120, "MusicPlayer");
	imagebuilder(115, 20, 99, 98, DrumPractice);
	LCD_DrawString(120, 120, "DrumPractice");
	imagebuilder(215, 20, 99, 98, Metronome);
	LCD_DrawString(230, 120, "Metronome");
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

/**
 * 	Important General Functions
 */

// Whenever touchscreen is pressed, this function is run
static void InterfaceSelector(int xpos, int ypos, int currentinterface) {
//	if ((xpos > 260) && (ypos > 200) // this can be replaced by a physical button
//			&& (!GUIEMPTYSTACK(GUISTACK))) {
//		GUIBACKWARD(GUISTACK);
//	} else {
		if (currentinterface == GUI_MainMenu) { // in the main menu, choose the different modes
			if (boundarychecker(xpos, ypos, 0, 110, 0, 120)) { // Mainmenu --> MusicPlayer
				GUIFORWARD(GUI_SongSelection, GUISTACK);
			} else if (boundarychecker(xpos, ypos, 115, 215, 0, 120)) { // MainMenu --> DrumPractice
				GUIFORWARD(GUI_DrumPractice, GUISTACK);
			} else if (boundarychecker(xpos, ypos, 215, 315, 0, 120)) { // MainMenu --> Metronome
				GUIFORWARD(GUI_Metronome, GUISTACK);
			}
		} else if (currentinterface == GUI_SongSelection) {
			GUIFORWARD(GUI_SongPlayer, GUISTACK);
			currentfile = FileSelector(xpos, ypos, 0, musicFileNum);
			setMusic(musicFilenames[currentfile]);
		} else if (currentinterface == GUI_SongPlayer) {
			MusicPlayerInterfaceSelector(xpos, ypos, 0);
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

//	}
}

// Displays the interface, will only run once whenever the touchscreen is pressed
static void DisplayInterface(int currentinterface) {

	switch (currentinterface) {
	case GUI_MainMenu: // MainMenu
		LCD_Clear(0, 0, 320, 240, WHITE);
		MainMenuInterface(); // Draws the main menu
		break;
	case GUI_SongSelection: // MusicPlayer
		LCD_Clear(0, 0, 320, 240, WHITE);
		SongSelectionInterface(musicFileNum, musicFilenames, 1); // Draws the files
		imagebuilder(260, 190, 31, 28, Return);
		break;
	case GUI_SongPlayer: // MusicPlayer-DrumPlay
		LCD_Clear(0, 0, 320, 240, WHITE);
		MusicPlayerInterface(0);
		//	 imagebuilder(260, 2100, 31, 28, Return);
		break;
	case GUI_DrumPractice:
		LCD_Clear(0, 0, 320, 240, WHITE);
		DrumPratice();
		break;
	case GUI_Metronome:
		LCD_Clear(0, 0, 320, 240, WHITE);
		imagebuilder(260, 210, 31, 28, Return);
		metronome(10);
		break;
	}

//	case 2: //FileFunctionMenu
//		//	  		  LCD_Clear(0,0,320,240,WHITE);
//		//	  		  if (filereturn ==0)
//		//	  		  {
//		//	  			  currentfile = FileSelector(posinfo[0],posinfo[1],0,4);
//		//	  		  }
//		//
////			  	  	  FileFunctionMenu(currentfile);
//		break;
//	case 4: // MusicPlayer-SoloPlay
//		//	  		  LCD_Clear(0,0,320,240,WHITE);
//		//	  		  filereturn =1;
//		break;

}

// Will always run in the main while loop
static uint32_t gui_last_tick = 0;
static void InterfaceHandler() {
	switch (currentinterface) {

	case GUI_SongPlayer:

		if (HAL_GetTick() - gui_last_tick > 1000) {
			gui_last_tick = HAL_GetTick();
			// update the timeline every second
			MusicTimeline(getMusicProgress(), 0);
		}

		// note: no need to call musicUpdate or drumUpdate here
		// it will be called by the main.c loop

		break;
	}
}

// Conventions
// _____Interface: call once, print the interface / update the interface (LCD_Draw, imgbuilder)
// _____Handler: run in a while loop, put any updating ADC/volume / music update function
// _____InterfaceSelector/Selector: xpos, ypos

#endif __GUI_H
