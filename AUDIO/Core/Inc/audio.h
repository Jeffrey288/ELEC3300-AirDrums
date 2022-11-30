#ifndef __AUDIO_H
#define __AUDIO_H
#include "recording.h"
#include "fileBuff.h"
#include "fatfs.h"
#include "drumSounds.h"

//#define DRUM_MACRO(X) \
//	X(KICK, "kick.wav") \
//	X(CRASH, "crash.wav") \
//	X(LOW_TOM, "tom1.wav") \
//	X(HIGH_TOM, "tom2.wav")

#define DRUM_MACRO(X) \
	X(KICK, kick) \
	X(CRASH, crash) \
	X(LOW_TOM, snare) \
	X(HIGH_TOM, tom3)

#define DRUM_ENUMS_DEF(a, ...) a,
//#define DRUM_FILENAME_DEF(a, b, ...) b,

typedef enum {
	DRUM_MACRO(DRUM_ENUMS_DEF)
	DRUM_NUM,
} DRUMS;

// -------------- MUSIC PLAYBACK --------------


typedef enum {
	MUSIC_UNINITED,
	MUSIC_PAUSED,
	MUSIC_PLAYING,
} MusicState;
extern MusicState musicState;

extern char musicFilenames[60][15];
extern char musicFilenamesLong[60][15];
extern uint16_t musicFileNum;
void addMusic(char* str);

int setMusic(char* fileName);
int playMusic();
int pauseMusic();
int stopMusic();
int seekMusic(float pos);
float getMusicProgress();
void musicUpdate();

// --------------- AUDIO FILE READING ---------------------
int drumMatch(char*);
int audioInit();
void drumPlay(DRUMS);
void drumUpdate();
extern FileStruct drumFileStructs[DRUM_NUM];
extern FileStruct sampleFile;
extern DRUMS activeDrums[DRUM_NUM];
extern int numActiveDrums;

#define drumMix(sampleSum) \
	for (int i = 0; i < numActiveDrums; i++) { \
		sampleSum += readSample(&drumFileStructs[activeDrums[i]]) / 3; \
	}

// ----------- AUDIO PLAYBACK -------------
#define AUDIO_FREQ		22050		// TIMER SETTING
#define SYSCLK_FREQ		72000000	// SYSCLOCK FREQUENCY
#define AUDIO_PRECOMP 	500		// precompute how many samples per interrupt
//#define AUDIO_PRECOMP 	500		// precompute how many samples per interrupt
//#define AUDIO_PRECOMP 	1000		// precompute how many samples per interrupt
// will hardfault if you set this to 1000
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
//extern AudioChannel audioRight; // unused for now

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern inline void audioChannelInit();
extern inline void precomputeMix();
extern inline void tempStopDMA();

#endif // __AUDIO_H
