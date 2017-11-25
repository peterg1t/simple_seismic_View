#ifndef FFTSHIFT_H
#define FFTSHIFT_H
#include <complex>


class fftshift
{
public:
    fftshift();
    fftshift_1D(std::complex<double> x[], int N);
    fftshift_2D(std::complex<double> x[], int N1, int N2);
};

#endif // FFTSHIFT_H
