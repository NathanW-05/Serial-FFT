#include "fft.h"
#include <stdio.h>

#define N 1024 // power of 2
#define sampling_rate 10000 // must be at least 2x dominant frequency of your signal

int main()
{
    float* signal = generate_test_signal(N, 1, 20, sampling_rate);
    Bin* bins = compute_fft(N, signal, sampling_rate);
    int peak = dominant_index(bins, N);
    printf("%f\n", bins[peak].frequency);
    return 0;
}

