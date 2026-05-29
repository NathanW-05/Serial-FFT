#include <stdio.h>
#include <stdint.h>

#include "fft.h"
#include "serial.h"
#include "include/raylib.h"

#define N 1024 // power of 2
#define FS 10000 // must be at least 2x dominant frequency of your signal
#define DATA_LEN_BYTES 2048
#define HEADER_LEN_BYTES 4
#define PACKETS_PER_WRITE 4
#define BASE_FREQ 24.125E9
#define C 3E8

uint8_t header[] = {0xA5, 0x5A, 0xFF, 0x00};

float max_speed(float freq) 
{ 
    return 2.2 * ((freq * (float)C) / (2.0f * (float)BASE_FREQ)); 
}

int main()
{
    init_serial("COM4", 1000000);
    begin_read_thread(header, HEADER_LEN_BYTES, DATA_LEN_BYTES, PACKETS_PER_WRITE);

    SetTraceLogLevel(LOG_NONE); 
    InitWindow(800, 450, "Serial FFT Data");

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Test", 190, 200, 20, BLACK);
        EndDrawing();
    }
    CloseWindow();

    while(1)
    {
        if(buffer_filled) // is there data available for processing?
        {
            // 4x1024 16 bit raw ADC values
            uint16_t** packets = parse_packet(read_buffer, PACKETS_PER_WRITE, HEADER_LEN_BYTES, DATA_LEN_BYTES);

            for(int p = 0; p < PACKETS_PER_WRITE; p++)
            {
                float *f_samples = (float*)malloc(N * sizeof(float));
                for(int i = 0; i < N; i++)
                {
                    f_samples[i] = (float)packets[p][i];
                }
                Bin* fft = compute_fft(N, f_samples, FS);
                free(packets[p]);
                free(f_samples);
                free(fft);
            }

            free(packets);
            swap_ready = 1; // swap the buffers, hopefully packets are waiting 
            buffer_filled = 0; // need to refill the old buffer with fresh data
        }
    }
    return 0;
}

