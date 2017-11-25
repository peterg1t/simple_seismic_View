#include "fft_complx.h"
#include <stdio.h>
#include <complex>
#include <qdebug.h>
#include <qtimer.h>
#include <QTime>
#include <QtMath>

fft_complx::fft_complx()
{
}


fft_complx::fft_fwd(std::complex<double> x[], int N)
{
    // DFT
    unsigned int k = N, n;
    double thetaT = qAcos(-1) / N;
    std::complex<double> phiT = {cos(thetaT), sin(thetaT)};
    std::complex<double> T, y[N];
    while (k > 1)
    {
        n = k;
        k >>= 1;
        phiT = phiT * phiT;
        T = 1.0;
        for (unsigned int l = 0; l < k; l++)
        {
            for (unsigned int a = l; a < N; a += n)
            {
                unsigned int b = a + k;
                std::complex<double> t = x[a] - x[b];
                x[a] += x[b];
                x[b] = t * T;
            }
            T *= phiT;
        }
    }
    // Decimate
    unsigned int m = (unsigned int)log2(N);
    for (unsigned int a = 0; a < N; a++)
    {
        unsigned int b = a;
        // Reverse bits
        b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
        b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
        b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
        b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
        b = ((b >> 16) | (b << 16)) >> (32 - m);
        if (b > a)
        {
            std::complex<double> t = x[a];
            x[a] = x[b];
            x[b] = t;
        }
    }
    for (unsigned int a = 0; a < N; a++){
    if(a==0){y[a]=x[a];}else{y[a]=x[N-a];}}
    for (unsigned int a = 0; a < N; a++){
    x[a]=y[a];
    }

    //// Normalize (This section make it not working correctly)
//    std::complex<double> f = 1.0 / sqrt(N);
//    for (unsigned int i = 0; i < N; i++)
//        x[i] *= f;



}



fft_complx::fft_bwd(std::complex<double> x[], int N)
{
    // DFT
    unsigned int k = N, n;
    double thetaT = 3.14159265358979323846264338328L / N;
    std::complex<double> phiT = {cos(thetaT), -sin(thetaT)};
    std::complex<double> T, y[N], OFF;
    while (k > 1)
    {
        n = k;
        k >>= 1;
        phiT = phiT * phiT;
        T = 1.0;
        for (unsigned int l = 0; l < k; l++)
        {
            for (unsigned int a = l; a < N; a += n)
            {
                unsigned int b = a + k;
                std::complex<double> t = x[a] - x[b];
                x[a] += x[b];
                x[b] = t * T;
            }
            T *= phiT;
        }
    }
    // Decimate
    unsigned int m = (unsigned int)log2(N);
    for (unsigned int a = 0; a < N; a++)
    {
        unsigned int b = a;
        // Reverse bits
        b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
        b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
        b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
        b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
        b = ((b >> 16) | (b << 16)) >> (32 - m);
        if (b > a)
        {
            std::complex<double> t = x[a];
            x[a] = x[b];
            x[b] = t;
        }
    }
    for (unsigned int a = 0; a < N; a++){
    if(a==0){y[a]=x[a];}else{y[a]=x[N-a];}}
    for (unsigned int a = 0; a < N; a++){
    x[a]=y[a];
    }



    std::complex<double> Per = {N,0};
    //// Here we calculate the offset introduced by the filter
    OFF=x[0]/Per;


    //// Normalize the reverse FFT
    for (unsigned int i = 0; i < N; i++)
    {
        x[i] = x[i]/Per;
        x[i]=x[i]-OFF;
    }

}










