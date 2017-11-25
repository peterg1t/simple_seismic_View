#ifndef WIGNERVILLE_H
#define WIGNERVILLE_H
#include <stdio.h>
#include <complex>
#include <QVector>
#include <vector>

class wignerville
{
public:
    wignerville();
    wignerville_fwd(std::complex<double> *x, QVector<double> &w, int tlength, int N);
    wignerville_fwd_kernel(std::complex<double> *x, QVector<double> &w, int tlength, int N);



};

#endif // WIGNERVILLE_H
