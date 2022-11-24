#ifndef __WAV_H__
#define __WAV_H__

#include "stm32f1xx_hal.h"
// Source: https://github.com/Nunocky/Nucleo_L476RG_WavPlay

//#pragma pack(1)
struct Wav_Header {
  char     riff[4];
  uint32_t fileSize;
  char     fileTypeHeader[4];
  char     formatChunkMarker[4];
  uint32_t formatChunkLength;
  uint16_t vfmt;
  uint16_t channels;
  uint32_t sampleFreq;
  uint32_t sampleBytesPerSecond;
  uint16_t blkSize;
  uint16_t bitsPerSample;
  char     dataChunkHeader[4];
  uint32_t dataChunkLength;
};
typedef struct Wav_Header WAV_HEADER;
typedef WAV_HEADER WavHeader;

#define WAV_HEADER_SIZE sizeof(WavHeader)

static const WavHeader emptyWavHeader = (WavHeader) {
  .riff = {'R', 'I', 'F', 'F'},
  .fileSize = 0,
  .fileTypeHeader = {'W', 'A', 'V', 'E'},
  .formatChunkMarker = {'f', 'm', 't', 32},
  .formatChunkLength = 16,
  .vfmt = 1,
  .channels = 1,
  .sampleFreq = 22050,
  .sampleBytesPerSecond = 44100,
  .blkSize = 2,
  .bitsPerSample = 16,
  .dataChunkHeader = {'d', 'a', 't', 'a'},
  .dataChunkLength = 0
};
// 9948302 - 9948160
// 12844174 - 12844032
// = 142

#endif

