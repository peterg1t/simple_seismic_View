#include "fftshift.h"
#include <complex>
#include <QDebug>

fftshift::fftshift()
{
}


fftshift::fftshift_1D(std::complex<double> x[], int N){
   std::complex<double> tmp;
    for(int i=0;i<N/2;i++){
      tmp=x[i];
      x[i]=x[N/2+i];
      x[N/2+i]=tmp;
    }
}


fftshift::fftshift_2D(std::complex<double> x[], int N1 , int N2){
    // N1 is the first dimension → and N2 is the second (row) dimension N2 ↓
    std::complex<double> tmp;
    for(int j=0;j<N2/2;j++){
       for(int i=0;i<N1/2;i++){
          tmp=x[(N2/2+j)*N1+N1/2+i];
          x[(N2/2+j)*N1+N1/2+i]=x[j*N1+i];
          x[j*N1+i]=tmp;
       }
       for(int i=0;i<N1/2;i++){
           tmp=x[(N2/2+j)*N1+i];
           x[(N2/2+j)*N1+i]=x[j*N1+N1/2+i];
           x[j*N1+N1/2+i]=tmp;
       }
    }
}
