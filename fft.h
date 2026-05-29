#ifndef FFT_H
#define FFT_H

#ifdef __cplusplus
extern "C" {
#endif

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
int noise_floor_magnitude(Bin* bin, int N);

#ifdef __cplusplus
}
#endif
#endif