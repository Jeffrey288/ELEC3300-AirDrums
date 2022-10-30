#include "fileBuff.h"

#ifndef __AUDIO_H
#define __AUDIO_H

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

#endif // __AUDIO_H
