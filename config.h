#ifndef CONFIG_H
#define CONFIG_H

#include "stdint.h"

#define FFT_N 512 // size of FFT
#define FS 20000 // sampling frequency
#define DATA_LEN_BYTES 1024 // length of packet data in bytes
#define HEADER_LEN_BYTES 4 // length of packet header in bytes
#define PACKETS_PER_WRITE 4 // number of packets to buffer
#define BASE_FREQ 24.125E9 // base frequency of radar
#define TIMEOUT 1 // ms to sleep while waiting for buffer filling
#define PORT "COM4"
#define BAUD_RATE 1000000

static const uint8_t header[] = {0xA5, 0x5A, 0xFF, 0x00};

#endif