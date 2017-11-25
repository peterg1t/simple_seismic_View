#include "fbpick.h"
#include <stdio.h>
#include <QVector>
#include <vector>
#include <QtMath>
#include <QDebug>
#include <QMessageBox>
#include <mainwindow.h>

extern int intsample;
extern quint32 tlength;
extern quint32 grp_fillen;

extern int shtTrmWnd; // short term window size
extern int EPSWndSz;  // EPS window size
extern double Beta;      // FB stability constant
extern int fbwStart;
extern int fbwEnd;
extern double fbThres;



fbpick::fbpick()
{

}



fbpick::fbpick_mcm(int grp_trpos, int grp_numtr, int grp_fbtype){


    if(fbwEnd-fbwStart<=shtTrmWnd){
        QMessageBox msgbox;
        msgbox.warning(0,"Warning","Short window length greater than window, please check your parameters");
    //        msgbox.setText("The file could not be opened");
    //        msgbox.exec();
            return;
    }



    if(grp_trpos>(grp_fillen-3200-400)/(240+4*tlength)){
        grp_trpos=(grp_fillen-3200-400)/(240+4*tlength);
    }



    if (grp_numtr + grp_trpos > (grp_fillen-3200-400)/(240+4*tlength)+1) {
        grp_numtr = (grp_fillen-3200-400)/(240+4*tlength)-grp_trpos+1;
    }


//    qDebug() << ui->tracePlot->graphCount();
//    double STA[shtTrmWnd];
    double ER[tlength];
    double w[EPSWndSz*EPSWndSz];
    double Mean[EPSWndSz];
    double Sum[EPSWndSz];
    double EPS[tlength]={};

    double STA;
    double LTA;

    QVector<double> timefb;
    QVector<double> fb;


    for(int l=0;l<ui->tracePlot->graphCount()/2;l++){

        QVector<double> StdDev(EPSWndSz);
        QVector<double> diff;

        for(int j=fbwStart;j<fbwEnd;j++){
            STA=0.0;
            LTA=0.0;
            for(int i=0;i<=j;i++){
              LTA=LTA+(ui->tracePlot->graph(2*l)->data().data()->at(i)->value-l*1.000005)*(ui->tracePlot->graph(2*l)->data().data()->at(i)->value-l*1.000005);
            }


            for(int i=j-shtTrmWnd;i<=j;i++){
              if(i>=0){
              STA=STA+(ui->tracePlot->graph(2*l)->data().data()->at(i)->value-l*1.000005)*(ui->tracePlot->graph(2*l)->data().data()->at(i)->value-l*1.000005);
              }
            }
          ER[j]=STA/(LTA+Beta);
//         if(l==0){qDebug() << j << ER[j];}
        }



        for(int k=fbwStart+EPSWndSz;k<fbwEnd-EPSWndSz;k++){
    //        EPS.insert(k,0.0);
            for(int i=0;i<EPSWndSz;i++){
                for(int j=k;j<k+EPSWndSz;j++){
                  w[i*EPSWndSz+j-k]=ER[j-EPSWndSz+i];
                }
            }
            for(int i=0;i<EPSWndSz;i++){
                Mean[i]=0.0;
                Sum[i]=0.0;
                StdDev.insert(i,0.0);
                for(int j=0;j<EPSWndSz;j++){
                    Mean[i]=Mean[i]+w[i*EPSWndSz+j]/EPSWndSz;
                }
                for(int j=0;j<EPSWndSz;j++){
                    Sum[i]=Sum[i]+(w[j*EPSWndSz+i]-Mean[i])*(w[j*EPSWndSz+i]-Mean[i])/(EPSWndSz-1);
                }
                StdDev.insert(i,std::sqrt(Sum[i]));
            }

            auto minVal=std::min_element(StdDev.begin(),StdDev.begin()+EPSWndSz);
            EPS[k]=Mean[std::distance(StdDev.begin(),minVal)];
        }




        for(int i=0;i<tlength-1;i++){
            diff.append((EPS[i+1]-EPS[i])/(intsample*1e-3));
//         if(l==0){qDebug() << i << /*intsample << EPS[i+1] << EPS[i] <<*/  diff.at(i);}
        }



        auto maxVal=std::max_element(diff.begin(),diff.end());
        int fbloc = std::distance(diff.begin(),maxVal);

        for(int i=0;i<tlength-1;i++){
           diff.replace(i,diff.at(i)/ *maxVal); //reescaling the difference vector will allow us to pick the first peak by adjusting our threshold
           if(diff.at(i)>=fbThres){
               fbloc=i;
               break;
           }
        }





        timefb.append(fbloc*intsample*1e-3);
        fb.append(ui->tracePlot->graph(2*l)->data().data()->at(fbloc)->value);


    }




}



fbpick::fbpick_fractal(int grp_trpos, int grp_numtr, int grp_fbtype){



}
