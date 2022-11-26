#ifndef __REC_H
#define __REC_H

#include "wav.h"
#include "audio.h"
#include "fatfs.h"

typedef enum {
	RecordingOff,
	RecordingOn,
} RecordingState;

extern RecordingState recState;

typedef struct {

	WavHeader head;
	FIL file;

	int16_t buff[5][500];
	int16_t toWrite;
	int16_t toRead;

} RecStruct;

extern RecStruct recStruct;
extern int16_t recBuff[500];

int startRecording();
extern inline int writeRecording(uint16_t len);
int endRecording();

#endif
