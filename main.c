#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "signal.h"
#include "serial.h"
#include "include/raylib.h"
#include "config.h"
#include "radar.h"

#define RAYGUI_IMPLEMENTATION
#include "include/raygui.h"

__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);

int main()
{
    // initialize raygui
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTraceLogLevel(LOG_NONE); 
    InitWindow(1000, 1000, "Serial FFT Control Panel");
    
    while(!WindowShouldClose())
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

        float sw = (float)GetScreenWidth();
        float sh = (float)GetScreenHeight();

        float padding = 20.0f;

        float header_height = 100.0f;
        Rectangle headerBounds = {padding, padding, sw - (padding * 2), header_height};

        float footer_height = 100.0f;
        Rectangle footerBounds = {padding, sh - footer_height - padding, sw - (padding * 2), footer_height};

        float centerTop = headerBounds.y + headerBounds.height + padding;
        float centerBottom = footerBounds.y - padding;
        Rectangle centerBounds = {padding, centerTop, sw - (padding * 2), centerBottom - centerTop};

        BeginDrawing();
        ClearBackground(RAYWHITE);

        /* Speed Label */
        GuiSetStyle(DEFAULT, TEXT_SIZE, 50);
        GuiLabel((Rectangle){headerBounds.x, headerBounds.y, headerBounds.width, headerBounds.height}, speed_text(get_last_speed()));
        GuiSetStyle(DEFAULT, TEXT_SIZE, 30);

        /* Serial Plot - ToDo*/
        GuiGroupBox(centerBounds, "Live Serial Plot");
        
        /* Calibration Button */
        int footer_element_width = 300;
        if(GuiButton((Rectangle){footerBounds.x, footerBounds.y, 300, footerBounds.height}, "Calibrate")) 
        {
            update_status(CALIBRATING);
        }

        GuiLabel((Rectangle){footerBounds.x+footer_element_width+padding, footerBounds.y, footerBounds.width, footerBounds.height}, status_text());

        /* Data Request & Processing */
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
        EndDrawing();
    }
    return 0;
}


