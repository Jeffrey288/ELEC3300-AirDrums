#include "audio.h"
#include "stm32f1xx_hal.h"
#include "fatfs.h"

//uint8_t drumInitFlags[DRUM_NUM] = {0};
//char drumFileNames[DRUM_NUM][20] = { DRUM_MACRO(DRUM_FILENAME_DEF) };

FileStruct drumFileStructs[DRUM_NUM];

// ----- ACTIVE DRUMS -------

DRUMS activeDrums[DRUM_NUM] = {-1};
int numActiveDrums = 0;

static inline void activateDrum(DRUMS index) {
	for (int i = 0; i < numActiveDrums; i++) {
		if (activeDrums[i] == index) return;
	}
	activeDrums[numActiveDrums++] = index;
}

static inline void deactivateDrum(DRUMS index) {
	int i = 0;
	for (; i < numActiveDrums && activeDrums[i] != index; i++); // find if index matches any drums
	if (i == numActiveDrums) return; // if cannot find it then return
	activeDrums[i] = activeDrums[--numActiveDrums]; // else, remove it from the list of drums
}

// ------------ MUSIC PLAYER --------------

MusicState musicState = MUSIC_UNINITED;
FileStruct sampleFile; // the file storing the music being played

char musicFilenames[60][15];
char musicFilenamesLong[60][15];
uint16_t musicFileNum = 0;

void addMusic(char* fileName) {
	strcpy(musicFilenames[musicFileNum++], fileName);
//	if (fil->lfname[0] == 0)
//		strcpy(musicFilenamesLong[musicFileNum++], fil->fname);
//	else
//		strcpy(musicFilenamesLong[musicFileNum++], fil->lfname);
}

int setMusic(char* fileName) {
	stopMusic();
	setFileName(&sampleFile, fileName);
	musicState = MUSIC_PAUSED;
	return openFile(&sampleFile);
}

int playMusic() {
	musicState = MUSIC_PLAYING;
}

int pauseMusic() {
	musicState = MUSIC_PAUSED;
}

int stopMusic() {
	f_close(&(sampleFile.file));
	musicState = MUSIC_UNINITED;
	return 0;
}

int seekMusic(float pos) {
	tempStopDMA();
	if (pos < 0 || pos >= 1) return -1;
	sampleFile.sampleCount = pos * sampleFile.totalSampleCount;
	return f_lseek(&(sampleFile.file), sizeof(WavHeader) + sampleFile.sampleCount * 2);
	// mistake here: cannot put
	// 		pos * sampleFile.sampleCount * 2
	// because this may return an odd number
}

float getMusicProgress() {
	if (sampleFile.totalSampleCount == 0) return 0;
	float res = (float) sampleFile.sampleCount / sampleFile.totalSampleCount;
	if (res > 1) res = 1;
	if (res < 0) res = 0;
	return res;
}

void musicUpdate() {
	readFile(&sampleFile);
}

// ---- DRUM INITING AND FILE MATCHING -----------

// Read the filenames, and match them to the files
// Returns -1 if there is no match, else returns the drum_num
int drumMatch(char *fileName) {
//	for (int i = 0; i < DRUM_NUM; i++) {
//		if (!drumInitFlags[i]) {
//			if (!strcmp(fileName, drumFileNames[i])) {
//				setFileName(&drumFileStructs[i], fileName);
//				drumInitFlags[i] = 1;
//				return i;
//			}
//		}
//	}
	return -1;
}


uint8_t sampleFileBuffs[BUFF_NUM][BUFF_SIZE];
#define DRUM_DEF(a, b, ...) setConstBuffPtr(&drumFileStructs[a], b, sizeof(b)/sizeof(uint16_t));
int audioInit() {

	DRUM_MACRO(DRUM_DEF)
	for (int i = 0; i < BUFF_NUM; i++) {
		sampleFile.structs[i].raw = sampleFileBuffs[i];
	}

//	for (int i = 0; i < DRUM_NUM; i++) {
//		if (!drumInitFlags[i]) return i;
//		if (openFile(&drumFileStructs[i]) != FR_OK) return i;
//		initFileStruct(&drumFileStructs[i]);
//	}

	return -1;
}

// -------------- DRUM PLAY -------------------

void drumPlay(DRUMS index) {
//	char buff[30];
//	sprintf(buff, "drum: %d", index);
//	LCD_DrawString(0, 160, buff);
//	openFile(&drumFileStructs[index]);
//	f_lseek(&(drumFileStructs[index].file), WAV_HEADER_SIZE);
	initFileStruct(&drumFileStructs[index]);
	drumFileStructs[index].sampleCount = 0;
	activateDrum(index);
}

void drumUpdate() {
	DRUMS temp[DRUM_NUM]; int temp_count = 0; // the drums that need to be deactivated
	for (int i = 0; i < numActiveDrums; i++) {
//		readFile(&drumFileStructs[activeDrums[i]]);
		if (!drumFileStructs[activeDrums[i]].inUse) {
			temp[temp_count++] = activeDrums[i];
		}
	}
	for (int i = 0; i < temp_count; i++) {
		deactivateDrum(temp[i]);
	}
}

// ------------ AUDIO PLAYBACK RELATED -----------------

const uint16_t AUDIO_BUFFSIZE = (AUDIO_PRECOMP * AUDIO_BLOCKS);
AudioChannel audioLeft;
//AudioChannel audioRight; // unused for now

inline void audioChannelInit() {
	// audioLeft and audioRight is for storing the precomputed mix
	for (int i = 0; i < AUDIO_BUFFSIZE; i++) {
		audioLeft.out[i] = 0;
//		audioRight.out[i] = 0;
	}
	audioLeft.toWrite = 0;
//	audioRight.toWrite = 0;
	audioLeft.curr = audioLeft.first = audioLeft.out;
//	audioRight.curr = audioRight.first = audioRight.out;
//	audioLeft.onFlag = 1;
//	audioRight.onFlag = 1;
	audioLeft.channel = DAC_CHANNEL_1;
//	audioRight.channel = DAC_CHANNEL_2;
	HAL_DAC_Start(&hdac, audioLeft.channel);
//	HAL_DAC_Start(&hdac, audioRight.channel);

	// The timers
	TIM4->ARR = SYSCLK_FREQ / AUDIO_FREQ - 1;
	TIM2->ARR = AUDIO_PRECOMP - 1;

	HAL_TIM_Base_Start(&htim4);
	HAL_TIM_Base_Start(&htim2);
	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
}

int16_t sample_sum;
int16_t playFlag = 0;
inline void precomputeMix() {
	if (recState == RecordingOff) {
		for (int i = 0; i < AUDIO_PRECOMP; i++) {
			sample_sum = 0;
			if (musicState == MUSIC_PLAYING)
				sample_sum += readSample(&sampleFile) / 4;
			drumMix(sample_sum);
			*(audioLeft.curr++) = (-sample_sum + 32768);
		}
	} else {
		for (int i = 0; i < AUDIO_PRECOMP; i++) {
			recStruct.buff[recStruct.toWrite][i]= 0;
			if (musicState == MUSIC_PLAYING)
				recStruct.buff[recStruct.toWrite][i] += readSample(&sampleFile) / 4;
			drumMix(recBuff[i]);
			*(audioLeft.curr++) = (-recStruct.buff[recStruct.toWrite][i] + 32768);
//			recStruct.buff[recStruct.toWrite][i] = (recStruct.buff[recStruct.toWrite][i] >> 8) | (recStruct.buff[recStruct.toWrite][i] << 8);
		}
		recStruct.toWrite = (recStruct.toWrite + 1) % 5;
	}
//	writeRecording(AUDIO_PRECOMP);
	audioLeft.toWrite++;
	if (audioLeft.toWrite >= AUDIO_BLOCKS) {
		audioLeft.curr = audioLeft.first;
		audioLeft.toWrite = 0;
	}

//	if (!(sampleFile.inUse)) stopMusic();
	if (!playFlag) {
		playFlag = 1;
		HAL_DAC_Start_DMA(&hdac, audioLeft.channel, (uint32_t*)audioLeft.out, AUDIO_BUFFSIZE, DAC_ALIGN_12B_L);
	}
//	if (!audioLeft.onFlag) {
//	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)dac_buff.left, AUDIO_PRECOMP, DAC_ALIGN_12B_L);
//		audioLeft.onFlag = 1;
//	}
}

inline void tempStopDMA() {
	__disable_irq();
	audioLeft.curr = audioLeft.first;
	audioLeft.toWrite = 1;
	playFlag = 0;
	HAL_DAC_Stop_DMA(&hdac, audioLeft.channel);
	__enable_irq();
}
