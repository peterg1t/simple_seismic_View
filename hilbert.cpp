#include "hilbert.h"
#include <stdio.h>
#include <complex>
#include <qdebug.h>
#include <qtimer.h>
#include <QTime>
#include <fft_complx.h>

hilbert::hilbert()
{
}


hilbert::hilbert_fwd(std::complex<double> x[], int N)
{
    double hv[N]={};
    fft_complx myfft;
    myfft.fft_fwd(x,N);


    for(int i=0;i<N;i++){    //Calculating the Heaviside transform
    if(i==0||i==N/2)  {hv[i]=1;}
    else if(i>0&&i<N/2)  {hv[i]=2;}
    else {hv[i]=0;}
      x[i]=hv[i]*x[i];
    }


    myfft.fft_bwd(x,N);
}



