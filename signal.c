#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "signal.h"

float* hann_window = NULL;

void init_hann_window() 
{
    hann_window = malloc(sizeof(float)*FFT_N);
    for (int i = 0; i < FFT_N; i++) 
    {
        hann_window[i] = 0.5f * (1.0f - cosf((2.0f * PI * i) / (FFT_N - 1)));
    }
}

static void fft_recursive(Complex* X, int _N)
{
    if(_N <= 1)
    {
        return;
    }

    Complex* even = (Complex*)malloc(_N/2*sizeof(Complex));
    Complex* odd = (Complex*)malloc(_N/2*sizeof(Complex));

    for(int i = 0; i < _N / 2; i++)
    {
        even[i] = X[2*i];
        odd[i] = X[2*i+1];
    }

    fft_recursive(even, _N/2);
    fft_recursive(odd, _N/2);

    for(int k = 0; k < _N/2; k++)
    {
        float angle = -2.0f * PI * (float)k / (float)_N;
        Complex twiddle = {cosf(angle), sinf(angle)};
        Complex t;
        t.real = odd[k].real * twiddle.real - odd[k].imag * twiddle.imag;
        t.imag = odd[k].real * twiddle.imag + odd[k].imag * twiddle.real;

        X[k].real = even[k].real + t.real;
        X[k].imag = even[k].imag + t.imag;

        X[k + _N/2].real = even[k].real - t.real;
        X[k + _N/2].imag = even[k].imag - t.imag;
    }
    free(even);
    free(odd);
}

Bin* compute_fft(float* signal)
{
    if(hann_window == NULL)
    {
        init_hann_window();
    }
    Complex* complex_signal = (Complex*)malloc(FFT_N * sizeof(Complex));
    Bin* output_bins = (Bin*)malloc(FFT_N * sizeof(Bin));

    for (int i = 0; i < FFT_N; i++) 
    {
        complex_signal[i].real = signal[i] * hann_window[i];
        complex_signal[i].imag = 0.0f;
    }

    fft_recursive(complex_signal, FFT_N);
    for (int k = 0; k < FFT_N; k++) 
    {
        output_bins[k].frequency = ((float)k * FS) / (float)FFT_N;
        
        float raw_mag = sqrtf(complex_signal[k].real * complex_signal[k].real + 
                              complex_signal[k].imag * complex_signal[k].imag);
        
        if (k == FFT_N / 2 || k == 0) 
        {
            output_bins[k].magnitude = raw_mag / (float)FFT_N;
        } else 
        {
            output_bins[k].magnitude = raw_mag / ((float)FFT_N / 2.0f);
        }
    }
    free(complex_signal);
    return output_bins;
}

float* generate_test_signal(float A, float F, float sampling_rate)
{
    float* signal = (float*)malloc(FFT_N*sizeof(float));
    for(int i = 0; i < FFT_N; i++)
    {
        double time = (float)i / sampling_rate;
        signal[i] = A*sinf(2*PI*F*time);
    }
    return signal;
}