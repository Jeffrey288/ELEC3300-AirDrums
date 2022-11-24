#ifndef __REC_H
#define __REC_H

#include "wav.h"
#include "fatfs.h"

typedef struct {
	WavHeader head;
	FIL file;
} RecStruct;

#endif
