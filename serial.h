#ifndef SERIAL_H
#define SERIAL_H

#include "config.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int init_serial(const char* port, int baud);
void begin_read_thread();
uint16_t** parse_packet(uint8_t* raw);
extern uint8_t* read_buffer;
extern uint8_t* write_buffer;
int is_serial_open();

extern volatile int swap_ready;
extern volatile int buffer_filled;

#endif