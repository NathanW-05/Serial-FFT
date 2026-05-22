#ifndef FFT_LIB_H
#define FFT_LIB_H

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

Bin* compute_fft(int N, float* signal, float sampling_rate);
float* generate_test_signal(int N, float A, float F, float sampling_rate);
int dominant_index(Bin* bins, int N);

#endif