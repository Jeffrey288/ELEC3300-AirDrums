#ifndef __AUDIO_H
#define __AUDIO_H
#include "fileBuff.h"

#define DRUM_MACRO(X) \
	X(KICK, "kick.wav") \
	X(CRASH, "crash.wav") \
	X(LOW_TOM, "tom1.wav") \
	X(HIGH_TOM, "tom2.wav")

#define DRUM_ENUMS_DEF(a, ...) a,
#define DRUM_FILENAME_DEF(a, b, ...) b,

typedef enum {
	DRUM_MACRO(DRUM_ENUMS_DEF)
	DRUM_NUM,
} DRUMS;

// --------------- AUDIO FILE READING ---------------------
void drumMatch(FILINFO* fno);
int audioInit();
void drumPlay(DRUMS);
void drumUpdate();
extern FileStruct drumFileStructs[DRUM_NUM];
extern FileStruct sampleFile;
extern DRUMS activeDrums[DRUM_NUM];
extern int numActiveDrums;

#define drumMix(sampleSum) \
	for (int i = 0; i < numActiveDrums; i++) { \
		sampleSum += readSample(&drumFileStructs[activeDrums[i]]) / 4; \
	}

// ----------- AUDIO PLAYBACK -------------
#define AUDIO_FREQ		22050		// TIMER SETTING
#define SYSCLK_FREQ		72000000	// SYSCLOCK FREQUENCY
#define AUDIO_PRECOMP 	500		// precompute how many samples per interrupt
#define AUDIO_BLOCKS	5		// how many audio blocks are there for DAC DMA
extern const uint16_t AUDIO_BUFFSIZE;

typedef struct {
	// the precomputed mix buffer that the DMA is going to send
	uint16_t out[AUDIO_PRECOMP * AUDIO_BLOCKS];
	uint16_t *curr;		// current position
	uint16_t *first;	// first position in the out array

	uint16_t toWrite;	// which block to write
	uint32_t channel;	// which DMA channel to use
	uint8_t onFlag;		// whether this audio channel is being used
	// UNUSED, the DMA will always be on for now
} AudioChannel;
extern AudioChannel audioLeft;
extern AudioChannel audioRight; // unused for now

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern inline void audioChannelInit();
extern inline void precomputeMix();

#endif // __AUDIO_H
