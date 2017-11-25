#ifndef FILTER_H
#define FILTER_H
#include <stdio.h>
#include <complex>
#include <QVector>
#include <vector>


class filter
{
public:
    filter();
    filter_butt(QVector<double> &w, int tlength, int ft);
    filter_orms(QVector<double> &w, int tlength);
};

#endif // FILTER_H
