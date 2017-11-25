#include "wignerville.h"
#include <stdio.h>
#include <complex>
#include <qdebug.h>
#include <qtimer.h>
#include <QTime>
#include <fft_complx.h>
#include <QVector>
#include <vector>
#include <math.h>
#include <QtMath>
#include <QFile>
#include "fftshift.h"


wignerville::wignerville()
{
}


wignerville::wignerville_fwd(std::complex<double> *x, QVector<double> &w, int tlength, int N)
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
//    for(int i=0;i<(int)std::ceil(tlength/2);i++){
    for(int i=0;i<tlength;i++){
        C[i]=sigwv[t+tlength+i]*sigwvconj[t+tlength-i];
       if(t>=i){C[i]=sigwv[t+tlength+i]*sigwvconj[t+tlength-i];}
       else   {C[i]={0,0};}
       if(i>=1&&i<=(int)std::round(tlength/2)-1){C[tlength-i]=std::conj(C[i]);}
     }


        fftwv.fft_fwd(C,N);  // computing the fft

       for(int i=0;i<N;i++){
        w[t*N+i]=std::real(C[i])/(2*N);
        C[i]={0,0};}
   }

//-----This was in the original Fortran code doing the fftshift------
//   for(int t=0;t<tlength;t++){
//       for(int i=0;i<N;i++){
//           w[i,t]=(-1.0)*w[i,t]*qPow(-1.0,std::real(i));
//       }
//   }


//   delete[] sigwv; // This block was causing problems
//   delete[] sigwvconj;
//   delete[] C;


}












wignerville::wignerville_fwd_kernel(std::complex<double> *x, QVector<double> &w, int tlength, int N)
{

//    QFile f("data_plot2.txt");
//    f.open(QFile::ReadWrite);
//    QTextStream stream(&f);

    std::complex<double> sigwv[3*tlength]={0,0};
    std::complex<double> sigwvconj[3*tlength]={0,0};
    std::complex<double> *wvd;
    wvd= new std::complex<double>[N*N*sizeof(std::complex<double>)];
//    wvd = (std::complex<double>*) malloc(N*N*sizeof(std::complex<double>));
    double ordn=4;
    double D0b = 25;



    for(int i=0;i<tlength;i++){
        sigwv[i+tlength]=x[i];
        sigwvconj[i+tlength]=std::conj(sigwv[i+tlength]);
    }


    std::complex<double> C[N] = {0,0};
    fft_complx fftwv;


//             computing the Wigner-Ville transform
   for(int t=0;t<tlength;t++){
    for(int i=0;i<tlength;i++){
//    for(int i=0;i<(int)std::ceil(tlength/2);i++){
        C[i]=sigwv[t+tlength+i]*sigwvconj[t+tlength-i];
       if(t>=i){C[i]=sigwv[t+tlength+i]*sigwvconj[t+tlength-i];}
       else   {C[i]={0,0};}
       if(i>=1&&i<=(int)std::round(tlength/2)-1){C[tlength-i]=std::conj(C[i]);}
     }


        fftwv.fft_fwd(C,N);  // computing the fft
        fftwv.fft_fwd(C,N);  // computing the fft

       for(int i=0;i<N;i++){
        wvd[t*N+i]=C[i];
        C[i]={0,0};}
       }


   for(int i=0;i<N;i++){
    for(int t=0;t<tlength;t++){
       C[t]=wvd[t*N+i];
       }
       fftwv.fft_fwd(C,N);  // computing the fft
       for(int t=0;t<tlength;t++){
       wvd[t*N+i]=C[t];
       C[t]={0,0};}
       }

   
   fftshift myfftshift;
   myfftshift.fftshift_2D(wvd,tlength,N);



//Applying the filter to the shifted data in the ambiguity space
   for(int t=0;t<tlength;t++){
       for(int i=0;i<N;i++){
         wvd[t*N+i]=wvd[t*N+i]*1./(1.+qPow(sqrt((i-N/2)*(i-N/2)+(t-tlength/2)*(t-tlength/2))/D0b,2*ordn));
        }
   }


   for(int i=0;i<N;i++){
    for(int t=0;t<tlength;t++){
       C[t]=wvd[t*N+i];
       }
       fftwv.fft_bwd(C,N);  // computing the reverse fft
       for(int t=0;t<tlength;t++){
       wvd[t*N+i]=C[t];
       C[t]={0,0};}
       }


   for(int t=0;t<tlength;t++){
    for(int i=0;i<N;i++){
       C[i]=wvd[t*N+i];
       }
       fftwv.fft_bwd(C,N);  // computing the reverse fft
       for(int i=0;i<N;i++){
       w[t*N+i]=std::real(C[i])/(4*N*N);
       C[i]={0,0};}
       }





//-----This was in the original Fortran code doing the fftshift------
//   for(int t=0;t<tlength;t++){
//       for(int i=0;i<N;i++){
//           w[i,t]=(-1.0)*w[i,t]*qPow(-1.0,std::real(i));
//       }
//   }



//   delete[] sigwv;
//   delete[] sigwvconj;
//   delete[] C;
   delete[] wvd;


}



