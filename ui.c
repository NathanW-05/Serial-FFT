#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "radar.h"
#include "ui.h"
#include "include/raylib.h"
#include "radar.h"
#include "config.h"
#include "signal.h"

#define RAYGUI_IMPLEMENTATION
#include "include/raygui.h"

void init_raylib()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTraceLogLevel(LOG_NONE); 
    InitWindow(800, 800, "Serial FFT Control Panel");
}

const char* speed_text(float speed)
{
    static char buffer[48]; 
    
    float rounded = roundf(speed * 10.0f) / 10.0f;
    if(speed > -1.0f)
    {
        snprintf(buffer, sizeof(buffer), "Speed: %.1f", rounded);
    } else
    {
        return "Speed: --.-";
    }
    return buffer;
}

const char* status_text()
{
    switch(get_status())
    {
        case DISCONNECTED: 
            return "Status: Disconnected";
        case IDLE:
            return "Status: Awaiting Calibration";
        case CALIBRATING:
            return "Status: Calibrating...";
        case CALIBRATED:
            return "Status: Calibrated";
    }
    return "Status: --";
}

int should_return()
{
    return WindowShouldClose();
}

void render_loop()
{
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    float padding = 30.0f;

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

    GuiGroupBox(centerBounds, "Live Serial Plot");

    //float magnitude_scale = (centerBottom - centerTop) / 20;
    float frequency_scale = (float)(sw - padding * 2) / (FFT_N / 2);
    static Bin fft[FFT_N/2];

    if (get_fft_display_data(fft))
    {
        for (int b = 2; b < FFT_N / 2; b++)
        {
            float current_fp_x = centerBounds.x + (b * frequency_scale);
            float next_fp_x    = centerBounds.x + ((b + 1) * frequency_scale);
            int start_x = (int)current_fp_x;
            int next_x  = (int)next_fp_x;
            int width = next_x - start_x; 
            int height = fft[b].magnitude;
            if(height > centerBounds.height) height = centerBounds.height;
            int start_y = centerBounds.y + centerBounds.height - height;

            DrawRectangle(start_x, start_y, width, height, BLUE);
        }
    }
    

    /* Calibration Button */
    int footer_element_width = 300;
    if(GuiButton((Rectangle){footerBounds.x, footerBounds.y, 300, footerBounds.height}, "Calibrate")) 
    {
        update_status(CALIBRATING);
    }

    GuiLabel((Rectangle){footerBounds.x+footer_element_width+padding, footerBounds.y, footerBounds.width, footerBounds.height}, status_text());
    EndDrawing();
}