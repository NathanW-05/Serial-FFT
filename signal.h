#ifndef FFT_H
#define FFT_H

#define PI 3.1415926535
#define C 3E8

#include "config.h"
typedef struct 
{
    float frequency;
    float magnitude;
} Bin;

typedef struct 
{
    float real;
    float imag;
} Complex;

Bin* compute_fft(float* signal);
float* generate_test_signal(float A, float F, float sampling_rate);

#endif