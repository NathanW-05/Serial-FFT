#include <stdio.h>
#include <stdint.h>

#include "fft.h"
#include "serial.h"

#define N 1024 // power of 2
#define FS 10000 // must be at least 2x dominant frequency of your signal
#define DATA_LEN_BYTES 2048
#define HEADER_LEN_BYTES 4
#define PACKETS_PER_WRITE 4

uint8_t header[] = {0xA5, 0x5A, 0xFF, 0x00};

int main()
{
    HANDLE serial_handle;
   
    serial_handle = init_serial("COM4", 1000000); // 1,000,000 baud rate, probably doesn't need to be higher
    begin_read_thread(serial_handle, header, HEADER_LEN_BYTES, DATA_LEN_BYTES, PACKETS_PER_WRITE);

    while(1)
    {
        if(buffer_filled) // is there data available for processing?
        {
            // 4x1024 16 bit raw ADC values
            uint16_t** packets = parse_packet(read_buffer, PACKETS_PER_WRITE, HEADER_LEN_BYTES, DATA_LEN_BYTES);
            for(int p = 0; p < PACKETS_PER_WRITE; p++)
            {
                float *f_samples = malloc(N * sizeof(float));
                for(int i = 0; i < N; i++)
                {
                    f_samples[i] = (float)packets[p][i];
                }
                Bin* fft = compute_fft(N, f_samples, FS);
                int top_index = dominant_index(fft, N);
                printf("%f\n", fft[top_index].magnitude);
                free(packets[p]);
                free(f_samples);
            }
            free(packets);
            swap_ready = 1; // swap the buffers, hopefully packets are waiting 
            buffer_filled = 0; // need to refill the old buffer with fresh data
        }
    }
    return 0;
}

