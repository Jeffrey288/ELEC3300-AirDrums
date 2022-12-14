#ifndef __WAV_H__
#define __WAV_H__

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


#endif
