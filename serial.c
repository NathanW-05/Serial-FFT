#include "serial.h"

#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <windows.h>

volatile int swap_ready = 1;
volatile int buffer_filled = 0;

uint8_t* write_buffer = NULL;
uint8_t* read_buffer = NULL;

static HANDLE handle = NULL;

int init_serial(const char* port, int baud_rate)
{
    handle = CreateFile(port, GENERIC_READ|GENERIC_WRITE, 0, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        //printf("Error setting serial handle\n");
        return 0;
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
        return 0;
    }
    dcbsp.BaudRate = baud_rate;
    dcbsp.ByteSize = 8;
    dcbsp.StopBits = ONESTOPBIT;
    dcbsp.Parity = NOPARITY;
    
    if (!SetCommState(handle, &dcbsp)) 
    {
        printf("Error setting device parameters\n");
        CloseHandle(handle);
        return 0;
    }
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(handle, &timeouts);
    return 1;
}

static void* begin_read(void* args)
{
    int target_headers = PACKETS_PER_WRITE+1;  // data after final header carries over yielding n-1 packets
    int buffer_size = (HEADER_LEN_BYTES + DATA_LEN_BYTES) * target_headers;
    
    write_buffer = (uint8_t*)malloc(buffer_size);
    read_buffer = (uint8_t*)malloc(buffer_size);
    
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

        BOOL ok = ReadFile(handle, write_buffer + accum_size, (buffer_size) - accum_size, &bytes_read, NULL);
        if(ok && bytes_read > 0)
        {
            for(DWORD i = 0; i < bytes_read; i++)
            {
                uint8_t byte = write_buffer[accum_size + i];
                if(byte == header[header_match])
                {
                    header_match++;
                    if(header_match == HEADER_LEN_BYTES)
                    {
                        packet_count++;
                        last_header_pos = accum_size + i - PACKETS_PER_WRITE - 1;
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
        } else 
        {
            DWORD last_error = GetLastError();
            if(last_error == ERROR_OPERATION_ABORTED)
            {
                handle = INVALID_HANDLE_VALUE;
                CloseHandle(handle);
                break;
            }
        }
    }
}

void begin_read_thread()
{
    pthread_t thread;

    pthread_create(&thread, NULL, begin_read, NULL);
    pthread_detach(thread);
}

int counter = 0;
uint16_t** parse_packet(uint8_t* raw)
{
    uint16_t** chunks = (uint16_t**)malloc(PACKETS_PER_WRITE * sizeof(uint16_t*));
    int jump = HEADER_LEN_BYTES + DATA_LEN_BYTES;
    
    for(int p = 0; p < PACKETS_PER_WRITE; p++)
    {
        uint8_t* payload = raw + (p * jump) + HEADER_LEN_BYTES;
        chunks[p] = (uint16_t*)malloc(DATA_LEN_BYTES);
        memcpy(chunks[p], payload, DATA_LEN_BYTES);

        //for(int i = 0; i < payload_size / 2; i++)
        //{
        //}
    }
    return chunks;
}

int is_serial_open()
{
    return (handle != INVALID_HANDLE_VALUE && handle != NULL);
}