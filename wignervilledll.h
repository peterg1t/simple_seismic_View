#ifndef WIGNERVILLEDLL_H
#define WIGNERVILLEDLL_H
#include <stdio.h>
#include <complex>
#include <QVector>
#include <vector>
#include <QtMath>

#include "wignervilledll_global.h"

class WIGNERVILLEDLLSHARED_EXPORT Wignervilledll
{

public:
    Wignervilledll();
    wignervilledll_fwd(std::complex<double> *x, QVector<double> &w, int tlength, int N);
    wignervilledll_fwd_kernel(std::complex<double> *x, QVector<double> &w, int tlength, int N);
    fft_fwd(std::complex<double> x[], int N);
    fft_bwd(std::complex<double> x[], int N);
    fftshift_1D(std::complex<double> x[], int N);
    fftshift_2D(std::complex<double> x[], int N1 , int N2);


};

#endif // WIGNERVILLEDLL_H
