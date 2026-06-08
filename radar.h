#ifndef STATE_H
#define STATE_H

#include "signal.h"

typedef enum
{
    DISCONNECTED,
    IDLE,
    UNCALIBRATED,
    CALIBRATING,
    CALIBRATED
} STATUS;

STATUS get_status();
void update_status(STATUS new);

void process_packets(uint16_t** packets);

float get_last_speed();
int get_fft_display_data(Bin* dest);

const char* speed_text(float speed);
const char* status_text();

#endif