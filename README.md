Program to read serial data and perform FFT analysis for radar

Requirements: mingw, 64 bit windows

build: gcc main.c fft.c serial.c -o main -lpthread -I include -L lib -lraylib -lgdi32 -lwinmm
