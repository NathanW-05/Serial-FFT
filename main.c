#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "signal.h"
#include "serial.h"
#include "config.h"
#include "radar.h"
#include "ui.h"

__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);

int main()
{
    init_raylib();
    while(!should_return())
    {
        if(!is_serial_open())
        {
            if(init_serial(PORT, BAUD_RATE))
            {
                update_status(IDLE);
                begin_read_thread();
            } else {
                update_status(DISCONNECTED);
            }
        }
        if(buffer_filled) // is there data available for processing?
        {
            if(get_status() != DISCONNECTED)
            {
                uint16_t** packets = parse_packet(read_buffer);
                process_packets(packets);
                for(int p = 0; p < PACKETS_PER_WRITE; p++)
                {
                    free(packets[p]);
                }
                free(packets);
            }
            swap_ready = 1;
            buffer_filled = 0;
        } else
        {
            Sleep(TIMEOUT);
        }
        render_loop();
    }
    return 0;
}


