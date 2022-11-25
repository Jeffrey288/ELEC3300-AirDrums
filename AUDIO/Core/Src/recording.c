#include <recording.h>

RecStruct recStruct = {
	.head = emptyWavHeader
};
int16_t recBuff[500];

int startRecording() {
	if (recState == RecordingOn) return -1;
	recState = RecordingOn;
//	char buff[30];
//	FILINFO fno;
//	DIR dir;
//	int i = 0;
//	do {
//		i++;
//		sprintf(buff, "Rec%d.wav", i);
//		f_findfirst(&dir, &fno, "", buff);
//	} while (fno.fname[0] != 0);
//	FRESULT res;
//	res = f_open(&(recStruct.file), buff, FA_CREATE_ALWAYS | FA_WRITE);
//	if (res != FR_OK) return res;
//	recStruct.head.dataChunkLength = 0;
//	uint32_t bytesWritten;
//	res = f_write(&(recStruct.file), (uint8_t*) &(recStruct.head), sizeof(recStruct.head), &bytesWritten);
	return -1;
}

inline int writeRecording(uint16_t len) {
	if (recState == RecordingOff) return -1;
	uint32_t bytesWritten;
	FRESULT res = f_write(&(recStruct.file), recBuff, len, &bytesWritten);
	recStruct.head.dataChunkLength += bytesWritten;
	return res;
}

int endRecording() {
	if (recState == RecordingOff) return -1;
	recState = RecordingOff;
	f_rewind(&(recStruct.file));
	uint32_t bytesWritten;
	recStruct.head.fileSize = recStruct.head.dataChunkLength + 142; // idk why but sure
	f_write(&(recStruct.file), (uint8_t*) &(recStruct.head), sizeof(recStruct.head), &bytesWritten);
	return f_close(&(recStruct.file));
}
