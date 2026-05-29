#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t* header;
    int header_len;
    int data_len;
    int packets_per_write;
} ReadArgs;

void init_serial(const char* port, int baud);
void begin_read_thread(uint8_t* header, int header_len, int data_length, int packets_per_write);
uint16_t** parse_packet(uint8_t* raw, int packet_count, int header_size, int payload_size);
extern uint8_t* read_buffer;
extern uint8_t* write_buffer;

extern volatile int swap_ready;
extern volatile int buffer_filled;

#ifdef __cplusplus
}
#endif

#endif