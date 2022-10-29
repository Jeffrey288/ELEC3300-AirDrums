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
int16_t drumMix();
extern FileStruct drumFileStructs[DRUM_NUM];
extern FileStruct sampleFile;

#endif // __AUDIO_H
