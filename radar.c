#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "radar.h"
#include "config.h"

STATUS current_status = DISCONNECTED;

float last_speed = -1;
float g_bias = 0;

Bin ui_display_buffer[FFT_N / 2];
int display_data_available = 0;

void update_status(STATUS new)
{
    current_status = new;
}

STATUS get_status()
{
    return current_status;
}

float get_last_speed()
{
    return last_speed;
}

int get_fft_display_data(Bin* dest)
{
    if(!display_data_available) return 0;
    for(int i = 0; i < FFT_N / 2; i++)
    {
        dest[i] = ui_display_buffer[i];
    }
    return 1;
}

Bin run_CFAR(Bin* fft, float bias, int* hits)
{
    int gap = 5;
    int ref = 10;
    int num_passed = 0;
    int peak_frequency_index = 2;
    for(int c = 2; c < FFT_N / 2; c++)
    {
        int counted_ref = 0;
        float cut_mag = fft[c].magnitude;
        float ref_sum = 0; 
        for(int r = c - gap - ref; r < c - gap; r++)
        {
            if(r >= 2)
            {
                counted_ref++;
                ref_sum+= fft[r].magnitude;
            }
        }
        for(int r = c + gap; r < c + gap + ref; r++)
        {
            if(r < FFT_N / 2)
            {
                counted_ref++;
                ref_sum+=fft[r].magnitude;
            }
        }
        float ref_average = ref_sum / (counted_ref*2);
        float threshold = ref_average * bias;
        if(cut_mag > threshold)
        {
            num_passed++;
            peak_frequency_index = c;
        }
    }
    *hits = num_passed;
    return (Bin){fft[peak_frequency_index].frequency, fft[peak_frequency_index].magnitude};
}

int calibrate(Bin* fft, float bias_increment)
{
    static float bias = 10; // config
    static int consecutive_frames_empty = 0;
    int hits = 0;
    Bin peak = run_CFAR(fft, bias, &hits);
    if(hits > 0)
    {
        bias+=bias_increment;
        consecutive_frames_empty = 0;
    } else 
    {
        consecutive_frames_empty++;
        if(consecutive_frames_empty == 100) // config
        {
            g_bias = bias;
            update_status(CALIBRATED);
            consecutive_frames_empty = 0;
        }
    }
}

int detect_speed(Bin* fft)
{
    int hits = 0;
    Bin peak_idx = run_CFAR(fft, g_bias, &hits);
    float mph = 2.237 * ((peak_idx.frequency * (float)C) / (2.0f * (float)BASE_FREQ));
    if(mph > 10)
    {
        printf("target detected: %f  with mag: %f and number of peaks: %d\n", mph, peak_idx.magnitude, hits);
        return 1;
    }
    last_speed = mph;
    return 0;
}

void process_packets(uint16_t** packets)
{
    for(int p = 0; p < PACKETS_PER_WRITE; p++)
    {
        float *f_samples = (float*)malloc(FFT_N * sizeof(float));
        for(int i = 0; i < FFT_N; i++)
        {
            f_samples[i] = (float)packets[p][i];
        }
        Bin* fft = compute_fft(f_samples);

        switch(get_status())
        {
            case CALIBRATING:
            {
                calibrate(fft, 1);
                free(fft);
                break;
            }
            
            case CALIBRATED:
            {
                if(detect_speed(fft))
                {
                    memcpy(ui_display_buffer, fft, FFT_N/2);
                    display_data_available = 1;
                } 
                free(fft);
                break;
            } 
        }
        free(f_samples);
    }
}