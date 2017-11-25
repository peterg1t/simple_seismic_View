#include "filter.h"
#include <stdio.h>
#include <complex>
#include <QVector>
#include <vector>
#include <QtMath>
#include <QDebug>
#include <fft_complx.h>

extern int intsample;
extern double lowCutFreq;
extern double highCutFreq;
extern double lowCutSlope;
extern double highCutSlope;
extern double lowPassFreq;
extern double highPassFreq;


filter::filter()
{

}



filter::filter_butt(QVector<double> &w, int tlength, int ft){


int N=qNextPowerOfTwo(w.size());
std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
double filt[N]={0};
double filt2[N]={0};
QVector<double> freq(N/2);  // frequency array for display
double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



//                 qDebug()<<"Padded trace";
        for(int i=0;i<tlength;i++){
            sig[i]={w[i],0};
        }



        freq[0]=0;
        for(int i=1;i<N/2;i++){
            freq[i]=freq[i-1]+freqinc;
        }

//                     qDebug() << "filter type" << grp_filttype;


        switch (ft) {
        case 1:
         for(int i=1;i<N/2;i++){
             filt[i]=1.0/(1+std::pow(freq.at(i)/lowCutFreq,2*lowCutSlope/6));
//             floor(n * pow(10., d) + .5) / pow(10., d); /* round number n to d decimal points */
//                      qDebug() << i << freq.at(i) << std::real(filt[i]) << std::floor(std::real(filt[i]) * std::pow(10., 6) + .5) / std::pow(10., 6);
         }

         for(int i=N/2;i<N;i++){
             filt[i]=1.0/(1+std::pow(freq.at(N-i-1)/lowCutFreq,2*lowCutSlope/6));
 //                     qDebug() << i << freq.at(N-i-1) << std::real(butt[i]);
         }
            break;
        case 2:
         for(int i=1;i<N/2;i++){
             filt[i]=1-1.0/(1+std::pow(freq.at(i)/highCutFreq,2*highCutSlope/6));
//             qDebug() << i << freq.at(i) << std::real(filt[i]);
         }

         for(int i=N/2;i<N;i++){
             filt[i]=1-1.0/(1+std::pow(freq.at(N-i-1)/highCutFreq,2*highCutSlope/6));
//             qDebug() << i << freq.at(N-i-1) << std::real(butt[i]);
         }
            break;
        case 3:
            for(int i=1;i<N/2;i++){
                filt[i]=1.0/(1+std::pow(freq.at(i)/lowCutFreq,2*lowCutSlope/6));
//                     qDebug() << i << freq.at(i) << std::real(butt[i]);
            }

            for(int i=N/2;i<N;i++){
                filt[i]=1.0/(1+std::pow(freq.at(N-i-1)/lowCutFreq,2*lowCutSlope/6));
//                     qDebug() << i << freq.at(N-i-1) << std::real(butt[i]);
            }
            for(int i=1;i<N/2;i++){
                filt2[i]=1-1.0/(1+std::pow(freq.at(i)/highCutFreq,2*highCutSlope/6));
//                qDebug() << i << freq.at(i) << std::real(filt[i]);
            }

            for(int i=N/2;i<N;i++){
                filt2[i]=1-1.0/(1+std::pow(freq.at(N-i-1)/highCutFreq,2*highCutSlope/6));
//                     qDebug() << i << freq.at(N-i-1) << std::real(butt[i]);
            }
            for(int i=1;i<N;i++){
                    filt[i]=filt[i]*filt2[i];   //creating the bandpasss filter
                }
            break;
        default:
            break;
        }






        fft_complx myfft;                   // computing the fft
        myfft.fft_fwd(sig,N);



            for(int i=0;i<N;i++){
                sig[i]=sig[i]*filt[i];   //applying filter
            }


        myfft.fft_bwd(sig,N);

             //                 qDebug()<<"Padded trace";
        for(int i=0;i<tlength;i++){
          w[i]=std::real(sig[i]);
        }



}






filter::filter_orms(QVector<double> &w, int tlength){
    int N=qNextPowerOfTwo(w.size());
    std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
    std::complex<double> filtcomplx[N]={0,0};
    QVector<double> realfilt(0);
    double pi=qAcos(-1);
    double minfilt, maxfilt;


    //                 qDebug()<<"Padded trace";
            for(int i=0;i<tlength;i++){
                sig[i]={w[i],0};
            }


            filtcomplx[0]={(  ((pi*pi*highCutFreq*highCutFreq)/(pi*highCutFreq-pi*highPassFreq))  -  ((pi*pi*highPassFreq*highPassFreq)/(pi*highCutFreq-pi*highPassFreq))  )   -   (   ((pi*pi*lowPassFreq*lowPassFreq)/(pi*lowPassFreq-pi*lowCutFreq))  -  ((pi*pi*lowCutFreq*lowCutFreq)/(pi*lowPassFreq-pi*lowCutFreq))    ),0};
            for(int i=1;i<tlength;i++){
                filtcomplx[i]={(  ((pi*pi*highCutFreq*highCutFreq)/(pi*highCutFreq-pi*highPassFreq))*(qSin(pi*highCutFreq*i*intsample*1e-6)/(pi*highCutFreq*i*intsample*1e-6))*(qSin(pi*highCutFreq*i*intsample*1e-6)/(pi*highCutFreq*i*intsample*1e-6))  -  ((pi*pi*highPassFreq*highPassFreq)/(pi*highCutFreq-pi*highPassFreq))*(qSin(pi*highPassFreq*i*intsample*1e-6)/(pi*highPassFreq*i*intsample*1e-6))*(qSin(pi*highPassFreq*i*intsample*1e-6)/(pi*highPassFreq*i*intsample*1e-6))  )   -   (   ((pi*pi*lowPassFreq*lowPassFreq)/(pi*lowPassFreq-pi*lowCutFreq))*(qSin(pi*lowPassFreq*i*intsample*1e-6)/(pi*lowPassFreq*i*intsample*1e-6))*(qSin(pi*lowPassFreq*i*intsample*1e-6)/(pi*lowPassFreq*i*intsample*1e-6))  -  ((pi*pi*lowCutFreq*lowCutFreq)/(pi*lowPassFreq-pi*lowCutFreq))*(qSin(pi*lowCutFreq*i*intsample*1e-6)/(pi*lowCutFreq*i*intsample*1e-6))*(qSin(pi*lowCutFreq*i*intsample*1e-6)/(pi*lowCutFreq*i*intsample*1e-6))     ),0};
            }



            fft_complx signalfft;                   // computing the fft
            signalfft.fft_fwd(sig,N);


            fft_complx filterfft;
            filterfft.fft_fwd(filtcomplx,N);

            for(int i=0;i<N;i++){
                realfilt.append(std::real(filtcomplx[i]));
            }

             minfilt = *std::min_element(realfilt.begin(),realfilt.end());
             maxfilt = *std::max_element(realfilt.begin(),realfilt.end());

            for(int i=0;i<N;i++){
                sig[i]=sig[i]*(realfilt.at(i) - minfilt)/maxfilt;
            }


            signalfft.fft_bwd(sig,N);


            for(int i=0;i<tlength;i++){
              w[i]=std::real(sig[i]);
            }

}
