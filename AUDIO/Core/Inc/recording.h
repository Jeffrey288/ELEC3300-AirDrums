#ifndef __REC_H
#define __REC_H

#include "wav.h"
#include "audio.h"
#include "fatfs.h"

typedef enum {
	RecordingOff,
	RecordingOn,
} RecordingState;

static RecordingState recState = RecordingOff;

typedef struct {
	WavHeader head;
	FIL file;
} RecStruct;

extern RecStruct recStruct;
extern int16_t recBuff[500];

int startRecording();
extern inline int writeRecording(uint16_t len);
int endRecording();

#endif
