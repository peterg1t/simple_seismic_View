#ifndef FFT_COMPLX_H
#define FFT_COMPLX_H
#include <stdio.h>
#include <complex>

class fft_complx
{
public:
    fft_complx();
    fft_fwd(std::complex<double> x[], int N);
    fft_bwd(std::complex<double> x[], int N);


};

#endif // FFT_COMPLX_H
