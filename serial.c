#include "serial.h"
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

volatile int swap_ready = 1;
volatile int buffer_filled = 0;

uint8_t* write_buffer = NULL;
uint8_t* read_buffer = NULL;

int packet_count = 0;

HANDLE init_serial(const char* port, int baud_rate)
{
    HANDLE handle = CreateFile(port, GENERIC_READ|GENERIC_WRITE, 0, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        printf("Error setting serial handle\n");
        return NULL;
    }
    else 
    {
        printf("Serial handle successfully created\n");
    }

    DCB dcbsp = {0};
    dcbsp.DCBlength = sizeof(dcbsp);
    
    if (!GetCommState(handle, &dcbsp)) 
    {
        printf("Error getting device state\n");
        CloseHandle(handle);
        return NULL;
    }
    dcbsp.BaudRate = baud_rate;
    dcbsp.ByteSize = 8;
    dcbsp.StopBits = ONESTOPBIT;
    dcbsp.Parity = NOPARITY;
    
    if (!SetCommState(handle, &dcbsp)) 
    {
        printf("Error setting device parameters\n");
        CloseHandle(handle);
        return NULL;
    }
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(handle, &timeouts);
    return handle;
}

static void* begin_read(void* args)
{
    ReadArgs* arg = (ReadArgs*)args;

    int target_headers = arg->packets_per_write+1;  // data after final header carries over yielding n-1 packets
    int buffer_size = (arg->header_len + arg->data_len) * target_headers;
    
    write_buffer = malloc(buffer_size);
    read_buffer = malloc(buffer_size);
    
    int accum_size = 0;
    DWORD bytes_read = 0;
    int header_match = 0; // number of subsequent header matches
    int packet_count = 0;
    int last_header_pos = 0;

    while(1)
    {
        if((buffer_size) - accum_size == 0)
        {
            accum_size = 0;
            packet_count = 0;
            header_match = 0;
            last_header_pos = 0;
            continue;
        }

        BOOL ok = ReadFile(arg->handle, write_buffer + accum_size, (buffer_size) - accum_size, &bytes_read, NULL);
        if(ok && bytes_read > 0)
        {
            for(DWORD i = 0; i < bytes_read; i++)
            {
                uint8_t byte = write_buffer[accum_size + i];
                if(byte == arg->header[header_match])
                {
                    header_match++;
                    if(header_match == arg->header_len)
                    {
                        packet_count++;
                        last_header_pos = accum_size + i - 3;
                        header_match = 0;
                    }
                }
                else
                {
                    header_match = 0;
                }
            }
            accum_size += bytes_read;

            if(packet_count >= target_headers && swap_ready)
            {
                buffer_filled = last_header_pos;  // only up to last complete packet
                
                uint8_t* tmp = write_buffer;
                write_buffer = read_buffer;
                read_buffer = tmp;
                
                // carry over bytes from last header onwards
                int carry = accum_size - last_header_pos;
                memcpy(write_buffer, read_buffer + last_header_pos, carry);
                accum_size = carry;
                
                packet_count = 1;
                header_match = 0;
                last_header_pos = 0;
                swap_ready = 0;
            }
        }
    }
}

void begin_read_thread(HANDLE serial_handle, uint8_t* header, int header_len, int data_length, int packets_per_write)
{
    pthread_t thread;
    ReadArgs* args = (ReadArgs*)malloc(sizeof(ReadArgs));
    args->handle = serial_handle;
    args->header = header;
    args->header_len = header_len;
    args->data_len = data_length;
    args->packets_per_write = packets_per_write;

    pthread_create(&thread, NULL, begin_read, args);
    pthread_detach(thread);
}

int counter = 0;
uint16_t** parse_packet(uint8_t* raw, int packet_count, int header_size, int payload_size)
{
    uint16_t** chunks = malloc(packet_count * sizeof(uint16_t*));
    int jump = header_size + payload_size;
    
    for(int p = 0; p < packet_count; p++)
    {
        uint8_t* payload = raw + (p * jump) + header_size;
        chunks[p] = malloc(payload_size);
        memcpy(chunks[p], payload, payload_size);

        //for(int i = 0; i < payload_size / 2; i++)
        //{
        //}
    }
    return chunks;
}