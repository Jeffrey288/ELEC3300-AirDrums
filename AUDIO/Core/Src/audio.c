#include "audio.h"
#include "stm32f1xx_hal.h"

uint8_t drumInitFlags[DRUM_NUM] = {0};
char drumFileNames[DRUM_NUM][20] = { DRUM_MACRO(DRUM_FILENAME_DEF) };

FileStruct drumFileStructs[DRUM_NUM];
FileStruct sampleFile;

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

// ---- DRUM INITING AND FILE MATCHING -----------

void drumMatch(FILINFO* file) {
	for (int i = 0; i < DRUM_NUM; i++) {
		if (!drumInitFlags[i]) {
			if (!strcmp(file->fname, drumFileNames[i])) {
				setFileName(&drumFileStructs[i], file->fname);
				drumInitFlags[i] = 1;
				return;
			}
		}
	}
}

int audioInit() {
	for (int i = 0; i < DRUM_NUM; i++) {
		if (!drumInitFlags[i]) return i;
		if (openFile(&drumFileStructs[i]) != FR_OK) return i;
//		initFileStruct(&drumFileStructs[i]);
	}
	return -1;
}

// -------------------

void drumPlay(DRUMS index) {
//	openFile(&drumFileStructs[index]);
	f_lseek(&(drumFileStructs[index].file), WAV_HEADER_SIZE);
	initFileStruct(&drumFileStructs[index]);
	activateDrum(index);
}

void drumUpdate() {
	DRUMS temp[DRUM_NUM]; int temp_count = 0; // the drums that need to be deactivated
	for (int i = 0; i < numActiveDrums; i++) {
		readFile(&drumFileStructs[activeDrums[i]]);
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
AudioChannel audioRight; // unused for now

inline void audioChannelInit() {
	// audioLeft and audioRight is for storing the precomputed mix
	for (int i = 0; i < AUDIO_BUFFSIZE; i++) {
		audioLeft.out[i] = 0;
		audioRight.out[i] = 0;
	}
	audioLeft.toWrite = 0;
	audioRight.toWrite = 0;
	audioLeft.curr = audioLeft.first = audioLeft.out;
	audioRight.curr = audioRight.first = audioRight.out;
//	audioLeft.onFlag = 1;
//	audioRight.onFlag = 1;
	audioLeft.channel = DAC_CHANNEL_1;
	audioRight.channel = DAC_CHANNEL_2;
	HAL_DAC_Start(&hdac, audioLeft.channel);
	HAL_DAC_Start(&hdac, audioRight.channel);

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
	for (int i = 0; i < AUDIO_PRECOMP; i++) {
		sample_sum = 0;
		sample_sum += readSample(&sampleFile) / 4;
		drumMix(sample_sum);
		*(audioLeft.curr++) = (sample_sum + 32768);
	}
	audioLeft.toWrite++;
	if (audioLeft.toWrite >= AUDIO_BLOCKS) {
		audioLeft.curr = audioLeft.first;
		audioLeft.toWrite = 0;
	}

	if (!playFlag) {
		playFlag = 1;
		HAL_DAC_Start_DMA(&hdac, audioLeft.channel, (uint32_t*)audioLeft.out, AUDIO_BUFFSIZE, DAC_ALIGN_12B_L);
	}
//	if (!audioLeft.onFlag) {
//	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)dac_buff.left, AUDIO_PRECOMP, DAC_ALIGN_12B_L);
//		audioLeft.onFlag = 1;
//	}
}
