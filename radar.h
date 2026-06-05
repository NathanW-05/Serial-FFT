#ifndef STATE_H
#define STATE_H

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

const char* speed_text(float speed);
const char* status_text();

#endif