#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fft.h"

#define PI 3.1415926535

static void fft_recursive(Complex* X, int N)
{
    if(N <= 1)
    {
        return;
    }

    Complex* even = (Complex*)malloc(N/2*sizeof(Complex));
    Complex* odd = (Complex*)malloc(N/2*sizeof(Complex));

    for(int i = 0; i < N / 2; i++)
    {
        even[i] = X[2*i];
        odd[i] = X[2*i+1];
    }

    fft_recursive(even, N/2);
    fft_recursive(odd, N/2);

    for(int k = 0; k < N/2; k++)
    {
        float angle = -2.0f * PI * (float)k / (float)N;
        Complex twiddle = {cosf(angle), sinf(angle)};
        Complex t;
        t.real = odd[k].real * twiddle.real - odd[k].imag * twiddle.imag;
        t.imag = odd[k].real * twiddle.imag + odd[k].imag * twiddle.real;

        X[k].real = even[k].real + t.real;
        X[k].imag = even[k].imag + t.imag;

        X[k + N/2].real = even[k].real - t.real;
        X[k + N/2].imag = even[k].imag - t.imag;
    }
    free(even);
    free(odd);
}

Bin* compute_fft(int N, float* signal, float sampling_rate)
{
    Complex* complex_signal = (Complex*)malloc(N * sizeof(Complex));
    Bin* output_bins = (Bin*)malloc(N * sizeof(Bin));

    for (int i = 0; i < N; i++) 
    {
        complex_signal[i].real = signal[i];
        complex_signal[i].imag = 0.0f;
    }

    fft_recursive(complex_signal, N);

    for (int k = 0; k < N; k++) 
    {
        output_bins[k].frequency = ((float)k * sampling_rate) / (float)N;
        
        float raw_mag = sqrtf(complex_signal[k].real * complex_signal[k].real + 
                              complex_signal[k].imag * complex_signal[k].imag);
        
        if (k == N / 2 || k == 0) 
        {
            output_bins[k].magnitude = raw_mag / (float)N;
        } else 
        {
            output_bins[k].magnitude = raw_mag / ((float)N / 2.0f);
        }
    }

    free(complex_signal);
    return output_bins;
}

float* generate_test_signal(int N, float A, float F, float sampling_rate)
{
    float* signal = (float*)malloc(N*sizeof(float));
    for(int i = 0; i < N; i++)
    {
        double time = (float)i / sampling_rate;
        signal[i] = A*sinf(2*PI*F*time);
    }
    return signal;
}

int dominant_index(Bin* bins, int N)
{
    int idx = -1;
    for(int i = 2; i < N/2; i++)
    {
        if(bins[i].frequency > 700)
        {
            idx = (bins[i].magnitude > bins[idx].magnitude) ?
         i : idx;
        }
    }
    return idx;
}

int noise_floor_magnitude(Bin* bin, int N)
{
    int max_mag = 0;
    for(int i = 2; i < N; i++)
    {
        float current_mag = bin[i].magnitude;
        max_mag = (max_mag > current_mag) ? max_mag : current_mag;
    }
    return max_mag;
}