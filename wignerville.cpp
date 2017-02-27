#include "wignerville.h"
#include <stdio.h>
#include <complex>
#include <qdebug.h>
#include <qtimer.h>
#include <QTime>
#include <fft_complx.h>
#include <QVector>
#include <vector>


wignerville::wignerville()
{
}


wignerville::wignerville_1(std::complex<double> *x, QVector<double> &w, int tlength, int N)
{

    std::complex<double> sigwv[3*tlength]={0,0};
    std::complex<double> sigwvconj[3*tlength]={0,0};


    for(int i=0;i<tlength;i++){
        sigwv[i+tlength]=x[i];
        sigwvconj[i+tlength]=std::conj(sigwv[i+tlength]);
    }

    std::complex<double> C[N] = {0,0};
    fft_complx fftwv;

    //             computing the Wigner-Ville transform

   for(int t=0;t<tlength;t++){
    for(int i=0;i<(int)std::ceil(tlength/2);i++){
       if(t>=i){C[i]=sigwv[t+tlength+i]*sigwvconj[t+tlength-i];}
       else   {C[i]={0,0};}
       if(i>=1&&i<=(int)std::round(tlength/2)-1){C[tlength-i]=std::conj(C[i]);}
        }


        fftwv.fft_fwd(C,N);  // computing the fft

       for(int i=0;i<N;i++){
        w[t*N+i]=std::real(C[i]);}
       for(int i=0;i<N;i++){C[i]={0,0};}
   }

   delete[] sigwv;
   delete[] sigwvconj;
   delete[] C;


}



