#include "audio.h"

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
