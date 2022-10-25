#include "fatfs.h"
#include "wav.h"
#define BUFF_SIZE (1000)
#define BUFF_NUM (2)

/**
 * Behaviour of FileStruct
 *
 * Reading:
 * - if currReading is not empty (i.e. buffSize > 0), then read
 * - else if currReading is empty, first peek if next one (i+1)%BUFF_NUM is empty
 * 		- if it is, do not change anything
 * 		- else if it isn't, then go to next one
 * 			- reset the pointer of curr
 * 			- start reading until buffSize goes to 0
 *
 * Writing:
 * - if currWriting = currReading, then do not write
 * - else if currWriting is empty, then write into it
 * 		- set buffSize to read size
 * 		- set data.raw to the read data
 * 		- go to next one
 *
 * Regarding whole struct:
 * - if the file is empty, set fileEmpty to false
 * - if the file is not empty, or if it is empty but the buffs are not all empty, then set inUse to true
 * - a struct can be taken over if the struct is not inUse
 */

typedef struct {
	uint8_t raw[BUFF_SIZE];
	uint8_t *first;			// pointer to the start of the array, i.e. raw
	uint8_t *curr;			// pointer to the currently read element
	uint16_t buffSize;		// stores how much data is left unread
	// to get empty, do !buffSize
} FileBuff;

typedef struct {
	FIL file;
	WavHeader header;
	uint8_t fileEmpty;
	uint8_t inUse;
	FileBuff structs[BUFF_NUM];
	uint8_t currReading;
	uint8_t currWriting;
	uint32_t sampleCount;
} FileStruct;

#define fileStructEmpty(f, index) (!((f)->structs[index].buffSize))

//void initFileSturct(FileStruct*);
//int initFileHeader(FileStruct*);
//int readFile(FileStruct*);
//int16_t readSample(FileStruct*);

static inline void initFileStruct(FileStruct *fileStruct) {
	for (int i = 0; i < BUFF_NUM; i++) {
		fileStruct->structs[i].curr = fileStruct->structs[i].first = fileStruct->structs[i].raw;
		fileStruct->structs[i].buffSize = 0;
	}
	fileStruct->currReading = BUFF_NUM - 1; // set to use the first fileStruct
	fileStruct->currWriting = 0; // also set to use the first fileStruct
	fileStruct->inUse = 1;
	fileStruct->fileEmpty = 0;
	fileStruct->sampleCount = 0;
}

static inline int initFileHeader(FileStruct *fileStruct) {
	int bytes_read;
	return f_read(&(fileStruct->file), &(fileStruct->header), sizeof(WAV_HEADER), &bytes_read);
}

static inline int readFile(FileStruct* f) {
	if (f->fileEmpty) return -1;

	if (f->currWriting == f->currReading) return -1;
	if (fileStructEmpty(f, f->currWriting)) {
		FRESULT res = f_read(&(f->file), f->structs[f->currWriting].raw, BUFF_SIZE, &(f->structs[f->currWriting].buffSize));
		if (res != FR_OK || f->structs[f->currWriting].buffSize == 0) {
			f->fileEmpty = 1;
			return -1;
		}
		f->currWriting = (f->currWriting + 1) % BUFF_NUM;
	}
	return 0;
}

static inline uint16_t readSample(FileStruct* f) {
	uint16_t temp;
	if (!f->inUse) return 0;

	if (!fileStructEmpty(f, f->currReading)) {
		f->structs[f->currReading].buffSize -= 2;
		temp = (f->structs[f->currReading].curr[1] << 8) | f->structs[f->currReading].curr[0];
		f->structs[f->currReading].curr += 2;
		f->sampleCount++;
		return temp;
	} else if (!fileStructEmpty(f, (f->currReading + 1) % BUFF_NUM)) {
		f->currReading = (f->currReading + 1) % BUFF_NUM;
		f->structs[f->currReading].curr = f->structs[f->currReading].first;
		readSample(f);
	} else {
		if (f->fileEmpty) f->inUse = 0;
		return 0;
	}
}



