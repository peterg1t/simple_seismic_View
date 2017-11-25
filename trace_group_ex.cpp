#include "trace_group_ex.h"
#include "ui_trace_group_ex.h"
#include <QFile>
#include <QDebug>
#include <QBitArray>
#include <stdio.h>
#include <qcustomPlot.h>
#include <QVector>
#include <QVector2D>
#include <QMetaObject>
#include <complex>
#include <fft_complx.h>
#include <QStatusBar>
#include <paramdialog.h>
#include <filtdlg.h>
#include <filter.h>
#include <fbdlg.h>



extern QString modelname;
quint32 grp_fillen;
extern int intsample;
extern quint32 tlength;
extern int code;
extern int inittr;
extern int numtr;
int grp_trpos;
int grp_numtr;
double grp_gain;
double grp_clip;
int grp_filttype;
int grp_fbtype;
quint32 grp_filepos;
int grp_sign;
double grp_mantissa;
int grp_exponent;
QByteArray tr_group_temp;
QByteArray hd_group_temp;
QByteArray tr_group_lbyte[100];
int nmpoint;
int sp1g; //slope point 1 graph number
int sp2g; //slope point 2 graph number
double xsi;
double ysi;
double xse;
double yse;
double xsm;
double ysm;
extern int unit;


extern double lowCutFreq;
extern double lowCutSlope;
extern double highCutFreq;
extern double highCutSlope;
extern double lowPassFreq;
extern double highPassFreq;

int bi_g[69]={0};
int bl_g[69]={0};

extern int shtTrmWnd; // short term window size
extern int EPSWndSz;  // EPS window size
extern double Beta;      // FB stability constant
extern int fbwStart;
extern int fbwEnd;
extern double fbThres;

bool fbcalc=false;





// In this subroutine we need to scan a group of traces and show them.


trace_group_ex::trace_group_ex(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::trace_group_ex)
{
    ui->setupUi(this);
    setWindowTitle("Trace Group Explorer");


    bi_g[0]=1;
    bl_g[0]=4;
    for(int i = 1 ; i<7;i++){
    bi_g[i]=bi_g[i-1]+bl_g[i-1];
    bl_g[i]=4;
    }
    for(int i = 7 ; i < 11;i++){
    bi_g[i]=bi_g[i-1]+bl_g[i-1];
    bl_g[i]=2;
    }
    for(int i = 11 ; i < 19;i++){
    bi_g[i]=bi_g[i-1]+bl_g[i-1];
    bl_g[i]=4;
    }
    for(int i = 19 ; i < 21;i++){
    bi_g[i]=bi_g[i-1]+bl_g[i-1];
    bl_g[i]=2;
    }
    for(int i = 21 ; i < 25;i++){
    bi_g[i]=bi_g[i-1]+bl_g[i-1];
    bl_g[i]=4;
    }
    for(int i = 25 ; i < 53;i++){
    bi_g[i]=bi_g[i-1]+bl_g[i-1];
    bl_g[i]=2;
    }
    bi_g[53]=149;
    bl_g[53]=2;
    for(int i = 54 ; i < 69;i++){
    bi_g[i]=bi_g[i-1]+bl_g[i-1];
    bl_g[i]=2;
    }


    QFile f(modelname);
    QDataStream in(&f);
    f.open(QFile::ReadOnly);
    grp_fillen=f.size();
    f.close();
    grp_trpos=1;
    grp_numtr=1;
    grp_gain=1.0;
    grp_clip=0;
    grp_filttype=0;
    grp_traceread(grp_trpos,grp_numtr,grp_gain,grp_clip,grp_filttype);
    ui->tracePlot->rescaleAxes();
    ui->tracePlot->replot();
    connect(ui->tracePlot,&QCustomPlot::mouseMove, this, &trace_group_ex::slotMouseMove);

    paramsdlg = new paramdialog;
    paramsdlg->setModal(true);
    connect(paramsdlg,SIGNAL(applyParams(int,int,double,double,int)),this,SLOT(grp_traceread(int,int,double,double,int)));

    filterdlg = new filtdlg;
    filterdlg->setModal(true);
    connect(filterdlg,SIGNAL(applyParams(int,int,double,double,int)),this,SLOT(grp_traceread(int,int,double,double,int)));

    fbdialog = new fbdlg;
    fbdialog->setModal(true);
    connect(fbdialog,SIGNAL(applyfbParams(int,int,int)),this,SLOT(grp_tracefbpick(int,int,int)));

    ui->prof_drw->setCheckable(true);


    ui->label->setVisible(false);
    ui->slope_label->setVisible(false);
    ui->trspacingSpinBox->setVisible(false);






}



void trace_group_ex::on_fitall_clicked()
{
    ui->tracePlot->rescaleAxes();
    ui->tracePlot->replot();
}

void trace_group_ex::on_fitvert_clicked()
{
    ui->tracePlot->yAxis->rescale(true);
    ui->tracePlot->replot();
}

void trace_group_ex::on_fithor_clicked()
{
    ui->tracePlot->xAxis->rescale(true);
    ui->tracePlot->replot();
}





void trace_group_ex::grp_tracefbpick(int grp_trpos, int grp_numtr, int grp_fbtype){


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


    //plotting the first breaks
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    pen.setColor(QColor(255,0,0,255));
    QCPScatterStyle scatterstylefb(QCPScatterStyle::ssDiamond,QColor(255,0,0,255),QColor(255,0,0,255),10);


    if(fbcalc==false){
      ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
//          ui->tracePlot->graph()->setData(timefb,fb); //adding a graph
      ui->tracePlot->graph(ui->tracePlot->graphCount()-1)->setData(timefb,fb); //adding a graph
      ui->tracePlot->graph(ui->tracePlot->graphCount()-1)->setPen(pen);
      ui->tracePlot->graph(ui->tracePlot->graphCount()-1)->setScatterStyle(scatterstylefb);
      ui->tracePlot->graph(ui->tracePlot->graphCount()-1)->setLineStyle(QCPGraph::lsNone);
      ui->tracePlot->replot();
    }
    else{
      ui->tracePlot->graph(ui->tracePlot->graphCount()-1)->setData(timefb,fb); //adding a graph
      ui->tracePlot->graph(ui->tracePlot->graphCount()-1)->setPen(pen);
      ui->tracePlot->graph(ui->tracePlot->graphCount()-1)->setScatterStyle(scatterstylefb);
      ui->tracePlot->graph(ui->tracePlot->graphCount()-1)->setLineStyle(QCPGraph::lsNone);
      ui->tracePlot->replot();
    }

    fbcalc=true;



}








void trace_group_ex::grp_traceread(int grp_trpos,int grp_numtr, double grp_gain, double grp_clip, int grp_filttype)
{

    if(grp_trpos>(grp_fillen-3200-400)/(240+4*tlength)){
        grp_trpos=(grp_fillen-3200-400)/(240+4*tlength);
    }



    if (grp_numtr + grp_trpos > (grp_fillen-3200-400)/(240+4*tlength)+1) {
        grp_numtr = (grp_fillen-3200-400)/(240+4*tlength)-grp_trpos+1;
    }



//    qDebug() << "Entering trace read group" << grp_trpos << grp_numtr << (grp_fillen-3200-400)/(240+4*tlength)-1;

    QFile f(modelname);
    f.open(QFile::ReadOnly);
    QVector<double> trace(tlength),traceposi(tlength); //Initializing vector with trace
    QVector<double> time(tlength), timeposi(tlength); //Initializing vector time axis


    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    pen.setColor(QColor(1,1,1));
    QPen pennone;
    pennone.setStyle(Qt::NoPen);
    pennone.setWidth(1);
    pennone.setColor(QColor(0,0,255,255));

    ui->tracePlot->clearGraphs();
    ui->tracePlot->yAxis->setLabel("Time (ms)"); //adding labels
    ui->tracePlot->xAxis->setVisible(false);

    qDebug()<< "code=" << code;


    for(int l=0;l<grp_numtr;l++){

    switch (code) {
    case 0:   // Seismic Unix Files
    {
        grp_filepos=(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header


// This section calculates the bytes to display the header attributes (4-bytes words)
        for(int k=0;k<69;k++){
            for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
               hd_group_temp.append(thead[i]);
            }
            tr_group_lbyte[k]=hd_group_temp;
            hd_group_temp=0;
        }

//        for (int i=0; i<69; i++) {
//        if(bl_g[i]==2){qDebug() << i << bi_g[i] << QString::number(static_cast<int16_t>(tr_group_lbyte[i].toHex().toInt(nullptr,16))) ;}
//        else{ qDebug() << i << bi_g[i] << QString::number(tr_group_lbyte[i].toHex().toInt(nullptr,16)) ;}
//        }

        tlength=static_cast<int16_t>(tr_group_lbyte[38].toHex().toInt(nullptr,16));
        intsample=static_cast<int16_t>(tr_group_lbyte[39].toHex().toInt(nullptr,16));

        qDebug() << "length=" << tlength << "sample=" << intsample;

        QVector<double> trace(tlength),traceposi(tlength); //Initializing vector with trace
        QVector<double> time(tlength), timeposi(tlength); //Initializing vector time axis


        QByteArray btrace(f.read(4*tlength));
        int j =0;
        for (quint32 i = 0 ; i < 4*tlength ;i = i + 4) {
        tr_group_temp.append(btrace[i]);
        tr_group_temp.append(btrace[i+1]);
        tr_group_temp.append(btrace[i+2]);
        tr_group_temp.append(btrace[i+3]);
        tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1
        while(tr_group_temp.length()<32) {
         tr_group_temp.insert(0,"0"); // inserting leading zeroes
        }
        if(tr_group_temp.at(0)=='1')  grp_sign =-1; else grp_sign=1; // if bit 0 is 1 number is negative
        tr_group_temp.remove(0,1); //removing from element 0 with length 1
        //                                   // remove grp_sign bit



        QByteArray fraction = tr_group_temp.right(23);   //get the fractional part
        grp_mantissa=0;
        for(int i=0 ; i < 23; i++) {
            // iterate through the fraction to claculate the grp_mantissa
         if(fraction.at(i)=='1')     grp_mantissa += 1/pow(2,i+1);
        }


        grp_exponent=0;
        for(int i=0; i < tr_group_temp.length()-23; i++) {
            // iterate through the array to calculate the grp_exponent
        grp_exponent += (tr_group_temp.at(i)-48)*pow(2,7-i);
        }


        trace[j]=grp_gain*grp_sign*(1+grp_mantissa)*pow(2,grp_exponent-127);
        time[j]=j*intsample*1e-3;//just adding the time vector
        timeposi[j]=j*intsample*1e-3;




        tr_group_temp=0;
        grp_exponent=0;
        grp_mantissa=0;
        j=j+1;
        }




        if(grp_filttype!=0){
            filter myfilt;
            switch (grp_filttype) {
            case 1:

                myfilt.filter_butt(trace,tlength,grp_filttype);
                break;
            case 2:

                myfilt.filter_butt(trace,tlength,grp_filttype);
                break;
            case 3:

                myfilt.filter_butt(trace,tlength,grp_filttype);
                break;
            case 4:

                myfilt.filter_orms(trace,tlength);
                break;
            default:
                break;
            }
        }



//           clipping the traces
        for(int j=0;j<tlength;j++){
        if (grp_clip!=0){
         if(trace[j]>grp_clip){trace[j]=grp_clip;}
         else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
        }
        }



        for(int j=0; j<trace.size(); j++){
            if(trace[j]>0 && trace[j+1]<0){
             traceposi[j]=0;
             double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
             timeposi[j]=yp;
            }
            else if(trace[j-1] < 0 && trace[j] > 0){
              traceposi[j]=0;
              double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
              timeposi[j]=yp;
            }
            else if(trace[j] > 0 && trace[j+1] > 0){
             traceposi[j]=0;
            }
            else{
                traceposi[j]=trace[j];
            }
        }

        for(int j=0; j<trace.size(); j++){
            trace[j]=trace[j]+l*1.000005;
            traceposi[j]=traceposi[j]+l*1.000005;
//            qDebug() << trace[j];
        }








// This section plots the trace data
       ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
       ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
       ui->tracePlot->graph(2*l)->setPen(pen);

       ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
       ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
       ui->tracePlot->graph(2*l+1)->setPen(pennone);
       ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

       ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
       ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


       connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
       ui->tracePlot->yAxis->setRangeReversed(true);







    }
       break;
    case 1:
    {  //  start 4-byte IBM floating point



        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }



                      QByteArray btrace(f.read(4*tlength));

    //               This loop will make groups of 4 bytes
                     int j =0;
                     for (quint32 i = 0 ; i < 4*tlength ;i = i + 4) {
                     tr_group_temp.append(btrace[i]);
                     tr_group_temp.append(btrace[i+1]);
                     tr_group_temp.append(btrace[i+2]);
                     tr_group_temp.append(btrace[i+3]);
                     tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1
    //                 if(grp_trpos=0 && j==0) qDebug() << "Plotting binary for sample j=" << j << tr_group_temp;
//                     qDebug() << fixed << qSetRealNumberPrecision(8) << j << tr_group_temp;
//                     exit(0);
                     while(tr_group_temp.length()<32) {
                      tr_group_temp.insert(0,"0"); // inserting leading zeroes
                     }
                     if(tr_group_temp.at(0)=='1')  grp_sign =-1; else grp_sign=1; // if bit 0 is 1 number is negative
                     tr_group_temp.remove(0,1); //removing from element 0 with length 1
                     //                                   // remove grp_sign bit



                     QByteArray fraction = tr_group_temp.right(24);   //get the fractional part
                     grp_mantissa=0;
                     for(int k=0 ; k< 24; k++) {
                         // iterate through the fraction to claculate the grp_mantissa
                      if(fraction.at(k)=='1')     grp_mantissa += 1.0/(pow(2,k));
                     }

                     grp_exponent=0;

                     for(int k=0; k < tr_group_temp.length()-24; k++) {
                         // iterate through the array to calculate the grp_exponent
                     grp_exponent += (tr_group_temp.at(k)-48)*pow(2,6-k);
                     }

                     trace[j]=grp_gain*(grp_sign*(grp_mantissa/2)*pow(16,grp_exponent-64));
//                     qDebug() << j << trace[j];
                     time[j]=j*intsample*1e-3;//just adding the time vector
                     timeposi[j]=j*intsample*1e-3;

                     tr_group_temp=0;
                     grp_exponent=0;
                     grp_mantissa=0;
                     j=j+1;
                     }





                     if(grp_filttype!=0){
                         filter myfilt;
                         switch (grp_filttype) {
                         case 1:

                             myfilt.filter_butt(trace,tlength,grp_filttype);
                             break;
                         case 2:

                             myfilt.filter_butt(trace,tlength,grp_filttype);
                             break;
                         case 3:

                             myfilt.filter_butt(trace,tlength,grp_filttype);
                             break;
                         case 4:

                             myfilt.filter_orms(trace,tlength);
                             break;
                         default:
                             break;
                         }
                     }


    //           clipping the traces
               for(int j=0;j<tlength;j++){
               if (grp_clip!=0){
                if(trace[j]>grp_clip){trace[j]=grp_clip;}
                else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
               }
               }



                     for(int j=0; j<trace.size(); j++){
                         if(trace[j]>0 && trace[j+1]<0){
                          traceposi[j]=0;
                          double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                          timeposi[j]=yp;
                         }
                         else if(trace[j-1] < 0 && trace[j] > 0){
                           traceposi[j]=0;
                           double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                           timeposi[j]=yp;
                         }
                         else if(trace[j] > 0 && trace[j+1] > 0){
                          traceposi[j]=0;
                         }
                         else{
                             traceposi[j]=trace[j];
                         }
                     }

                     for(int j=0; j<trace.size(); j++){
                         trace[j]=trace[j]+l*1.000005;
                         traceposi[j]=traceposi[j]+l*1.000005;
                     }






     // This section plots the trace data
     ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
     ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
     ui->tracePlot->graph(2*l)->setPen(pen);

     ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
     ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
     ui->tracePlot->graph(2*l+1)->setPen(pennone);
     ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

     ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
     ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


     connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
     ui->tracePlot->yAxis->setRangeReversed(true);


        }   //  end 4-byte IBM floating point
        break;
    case 2:
    {// 4-byte two's complement integer
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }

            QByteArray btrace(f.read(4*tlength));

    //               This loop will make groups of 4 bytes
           int j =0;
           for (quint32 i = 0 ; i < 4*tlength ;i = i + 4) {
           tr_group_temp.append(btrace[i]);
           tr_group_temp.append(btrace[i+1]);
           tr_group_temp.append(btrace[i+2]);
           tr_group_temp.append(btrace[i+3]);
           tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1
    //                 if(grp_trpos=0 && j==0) qDebug() << "Plotting binary for sample j=" << j << tr_group_temp;
    //                 exit(0);
           while(tr_group_temp.length()<32) {
            tr_group_temp.insert(0,"0"); // inserting leading zeroes
           }
    //       if(tr_group_temp.at(0)=='1')  grp_sign =-1; else grp_sign=1; // No sign bit in this mode


           trace[j]= -(tr_group_temp.at(0)-48)*pow(2,31);
           for(int k=1; k<32; k++){
               trace[j]+=(tr_group_temp.at(k)-48)*qPow(2,31-k);
           }
           time[j]=j*intsample*1e-3;//just adding the time vector
           timeposi[j]=j*intsample*1e-3;

           tr_group_temp=0;
           j=j+1;
           }


           if(grp_filttype!=0){
               filter myfilt;
               switch (grp_filttype) {
               case 1:

                   myfilt.filter_butt(trace,tlength,grp_filttype);
                   break;
               case 2:

                   myfilt.filter_butt(trace,tlength,grp_filttype);
                   break;
               case 3:

                   myfilt.filter_butt(trace,tlength,grp_filttype);
                   break;
               case 4:

                   myfilt.filter_orms(trace,tlength);
                   break;
               default:
                   break;
               }
           }



    //           clipping the traces
               for(int j=0;j<tlength;j++){
               if (grp_clip!=0){
                if(trace[j]>grp_clip){trace[j]=grp_clip;}
                else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
               }
               }



           for(int j=0; j<trace.size(); j++){
               if(trace[j]>0 && trace[j+1]<0){
                traceposi[j]=0;
                double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                timeposi[j]=yp;
               }
               else if(trace[j-1] < 0 && trace[j] > 0){
                 traceposi[j]=0;
                 double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                 timeposi[j]=yp;
               }
               else if(trace[j] > 0 && trace[j+1] > 0){
                traceposi[j]=0;
               }
               else{
                   traceposi[j]=trace[j];
               }
           }

           for(int j=0; j<trace.size(); j++){
               trace[j]=trace[j]+l*1.000005;
               traceposi[j]=traceposi[j]+l*1.000005;
           }





           // This section plots the trace data
           ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
           ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
           ui->tracePlot->graph(2*l)->setPen(pen);

           ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
           ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
           ui->tracePlot->graph(2*l+1)->setPen(pennone);
           ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

           ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
           ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


           connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
           ui->tracePlot->yAxis->setRangeReversed(true);

    }// end 4-byte two's complement integer
        break;
    case 3:
    {   // 2-byte two's complement integer
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }

            QByteArray btrace(f.read(2*tlength));
    //               This loop will make groups of 2 bytes
            int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
            for (quint32 i = 0 ; i < 2*tlength ;i = i + 2) {
            tr_group_temp.append(btrace[i]);
            tr_group_temp.append(btrace[i+1]);
            tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

      //                 exit(0);
            while(tr_group_temp.length()<16) {
             tr_group_temp.insert(0,"0"); // inserting leading zeroes
            }

            trace[j]= -(tr_group_temp.at(0)-48)*pow(2,15);
            for(int k=1; k<16; k++){
                trace[j]+=(tr_group_temp.at(k)-48)*qPow(2,15-k);
            }
            time[j]=j*intsample*1e-3;//just adding the time vector
            timeposi[j]=j*intsample*1e-3;

            tr_group_temp=0;
            j=j+1;
            }




            if(grp_filttype!=0){
                filter myfilt;
                switch (grp_filttype) {
                case 1:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 2:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 3:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 4:

                    myfilt.filter_orms(trace,tlength);
                    break;
                default:
                    break;
                }
            }


    //           clipping the traces
             for(int j=0;j<tlength;j++){
             if (grp_clip!=0){
              if(trace[j]>grp_clip){trace[j]=grp_clip;}
              else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
             }
             }



             for(int j=0; j<trace.size(); j++){
                 if(trace[j]>0 && trace[j+1]<0){
                  traceposi[j]=0;
                  double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                  timeposi[j]=yp;
                 }
                 else if(trace[j-1] < 0 && trace[j] > 0){
                   traceposi[j]=0;
                   double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                   timeposi[j]=yp;
                 }
                 else if(trace[j] > 0 && trace[j+1] > 0){
                  traceposi[j]=0;
                 }
                 else{
                     traceposi[j]=trace[j];
                 }
             }

            for(int j=0; j<trace.size(); j++){
                trace[j]=trace[j]+l*1.000005;
                traceposi[j]=traceposi[j]+l*1.000005;
            }






            // This section plots the trace data
            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
            ui->tracePlot->graph(2*l)->setPen(pen);

            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
            ui->tracePlot->graph(2*l+1)->setPen(pennone);
            ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

            ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
            ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


            connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
            ui->tracePlot->yAxis->setRangeReversed(true);
        } //  end 2-byte two's complement integer
        break;
    case 4:
        {QByteArray trace(f.read(4*tlength));}  // 4-byte fixed-point with gain
        break;
    case 5:
    { // start 4-byte IEEE floating-point
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }
                QByteArray btrace(f.read(4*tlength));



    //               This loop will make groups of 4 bytes
               int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
               for (quint32 i = 0 ; i < 4*tlength ;i = i + 4) {
               tr_group_temp.append(btrace[i]);
               tr_group_temp.append(btrace[i+1]);
               tr_group_temp.append(btrace[i+2]);
               tr_group_temp.append(btrace[i+3]);
               tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1
               while(tr_group_temp.length()<32) {
                tr_group_temp.insert(0,"0"); // inserting leading zeroes
               }
               if(tr_group_temp.at(0)=='1')  grp_sign =-1; else grp_sign=1; // if bit 0 is 1 number is negative
               tr_group_temp.remove(0,1); //removing from element 0 with length 1
               //                                   // remove grp_sign bit



               QByteArray fraction = tr_group_temp.right(23);   //get the fractional part
               grp_mantissa=0;
               for(int i=0 ; i < 23; i++) {
                   // iterate through the fraction to claculate the grp_mantissa
                if(fraction.at(i)=='1')     grp_mantissa += 1/pow(2,i+1);
               }


               grp_exponent=0;
               for(int i=0; i < tr_group_temp.length()-23; i++) {
                   // iterate through the array to calculate the grp_exponent
               grp_exponent += (tr_group_temp.at(i)-48)*pow(2,7-i);
               }


               trace[j]=grp_gain*grp_sign*(1+grp_mantissa)*pow(2,grp_exponent-127);
    //           qDebug() << fixed << qSetRealNumberPrecision(8) << j << trace[j] << 1+grp_mantissa << grp_exponent;
               time[j]=j*intsample*1e-3;//just adding the time vector
               timeposi[j]=j*intsample*1e-3;//just adding the time vector


               tr_group_temp=0;
               grp_exponent=0;
               grp_mantissa=0;
               j=j+1;
               }


               if(grp_filttype!=0){
                   filter myfilt;
                   switch (grp_filttype) {
                   case 1:

                       myfilt.filter_butt(trace,tlength,grp_filttype);
                       break;
                   case 2:

                       myfilt.filter_butt(trace,tlength,grp_filttype);
                       break;
                   case 3:

                       myfilt.filter_butt(trace,tlength,grp_filttype);
                       break;
                   case 4:

                       myfilt.filter_orms(trace,tlength);
                       break;
                   default:
                       break;
                   }
               }


    //           clipping the traces
               for(int j=0;j<tlength;j++){
               if (grp_clip!=0){
                if(trace[j]>grp_clip){trace[j]=grp_clip;}
                else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
               }
               }


               for(int j=0; j<trace.size(); j++){
                   if(trace[j]>0 && trace[j+1]<0){
                    traceposi[j]=0;
                    double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                    timeposi[j]=yp;
                   }
                   else if(trace[j-1] < 0 && trace[j] > 0){
                     traceposi[j]=0;
                     double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                     timeposi[j]=yp;
                   }
                   else if(trace[j] > 0 && trace[j+1] > 0){
                    traceposi[j]=0;
                   }
                   else{
                       traceposi[j]=trace[j];
                   }
               }

               for(int j=0; j<trace.size(); j++){
                   trace[j]=trace[j]+l*1.000005;
                   traceposi[j]=traceposi[j]+l*1.000005;
               }




               // This section plots the trace data
               ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
               ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
               ui->tracePlot->graph(2*l)->setPen(pen);

               ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
               ui->tracePlot->graph(2*l+1)->setData(time,trace); //adding a graph
               ui->tracePlot->graph(2*l+1)->setPen(pennone);
               ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

               ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
               ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));
               connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
               ui->tracePlot->yAxis->setRangeReversed(true);

            }  // end 4-byte IEEE floating-point
        break;
    case 6:
    { // start 8-byte IEEE floating-point
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }
                QByteArray btrace(f.read(8*tlength));



    //               This loop will make groups of 4 bytes
               int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
               for (quint32 i = 0 ; i < 8*tlength ;i = i + 8) {
               tr_group_temp.append(btrace[i]);
               tr_group_temp.append(btrace[i+1]);
               tr_group_temp.append(btrace[i+2]);
               tr_group_temp.append(btrace[i+3]);
               tr_group_temp.append(btrace[i+4]);
               tr_group_temp.append(btrace[i+5]);
               tr_group_temp.append(btrace[i+6]);
               tr_group_temp.append(btrace[i+7]);
               tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1
               while(tr_group_temp.length()<64) {
                tr_group_temp.insert(0,"0"); // inserting leading zeroes
               }

               if(tr_group_temp.at(0)=='1')  grp_sign =-1; else grp_sign=1; // if bit 0 is 1 number is negative
               tr_group_temp.remove(0,1); //removing from element 0 with length 1
               //                                   // remove grp_sign bit



               QByteArray fraction = tr_group_temp.right(52);   //get the fractional part
    //           qDebug() << fraction;
               grp_mantissa=0;
               for(int i=0 ; i < 52; i++) {
                   // iterate through the fraction to claculate the grp_mantissa
                if(fraction.at(i)=='1')     grp_mantissa += 1.0/(pow(2,i+1));
               }

               grp_exponent=0;

               for(int i=0; i < tr_group_temp.length()-52; i++) {
                   // iterate through the array to calculate the grp_exponent
               grp_exponent += (tr_group_temp.at(i)-48)*pow(2,10-i);
               }


               trace[j]=grp_gain*grp_sign*(1+grp_mantissa)*pow(2,grp_exponent-1023);
//               qDebug() << fixed << qSetRealNumberPrecision(8) << j << trace[j] << 1+grp_mantissa << grp_exponent;
               time[j]=j*intsample*1e-3;//just adding the time vector
               timeposi[j]=j*intsample*1e-3;//just adding the time vector


               tr_group_temp=0;
               grp_exponent=0;
               grp_mantissa=0;
               j=j+1;
               }


               if(grp_filttype!=0){
                   filter myfilt;
                   switch (grp_filttype) {
                   case 1:

                       myfilt.filter_butt(trace,tlength,grp_filttype);
                       break;
                   case 2:

                       myfilt.filter_butt(trace,tlength,grp_filttype);
                       break;
                   case 3:

                       myfilt.filter_butt(trace,tlength,grp_filttype);
                       break;
                   case 4:

                       myfilt.filter_orms(trace,tlength);
                       break;
                   default:
                       break;
                   }
               }


    //           clipping the traces
               for(int j=0;j<tlength;j++){
               if (grp_clip!=0){
                if(trace[j]>grp_clip){trace[j]=grp_clip;}
                else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
               }
               }


               for(int j=0; j<trace.size(); j++){
                   if(trace[j]>0 && trace[j+1]<0){
                    traceposi[j]=0;
                    double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                    timeposi[j]=yp;
                   }
                   else if(trace[j-1] < 0 && trace[j] > 0){
                     traceposi[j]=0;
                     double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                     timeposi[j]=yp;
                   }
                   else if(trace[j] > 0 && trace[j+1] > 0){
                    traceposi[j]=0;
                   }
                   else{
                       traceposi[j]=trace[j];
                   }
               }

               for(int j=0; j<trace.size(); j++){
                   trace[j]=trace[j]+l*1.000005;
                   traceposi[j]=traceposi[j]+l*1.000005;
               }




               // This section plots the trace data
               ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
               ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
               ui->tracePlot->graph(2*l)->setPen(pen);

               ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
               ui->tracePlot->graph(2*l+1)->setData(time,trace); //adding a graph
               ui->tracePlot->graph(2*l+1)->setPen(pennone);
               ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

               ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
               ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));
               connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
               ui->tracePlot->yAxis->setRangeReversed(true);


            }  // end 8-byte IEEE floating-point
        break;
    case 7:
    {   // 3-byte two's complement integer
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }
            QByteArray btrace(f.read(3*tlength));
    //               This loop will make groups of 2 bytes
            int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
            for (quint32 i = 0 ; i < 3*tlength ;i = i + 3) {
            tr_group_temp.append(btrace[i]);
            tr_group_temp.append(btrace[i+1]);
            tr_group_temp.append(btrace[i+2]);
            tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

      //                 exit(0);
            while(tr_group_temp.length()<24) {
             tr_group_temp.insert(0,"0"); // inserting leading zeroes
            }

            trace[j]= -(tr_group_temp.at(0)-48)*pow(2,23);
            for(int k=1; k<24; k++){
                trace[j]+=(tr_group_temp.at(k)-48)*qPow(2,23-k);
            }
            time[j]=j*intsample*1e-3;//just adding the time vector
            timeposi[j]=j*intsample*1e-3;

            tr_group_temp=0;
            j=j+1;
            }




            if(grp_filttype!=0){
                filter myfilt;
                switch (grp_filttype) {
                case 1:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 2:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 3:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 4:

                    myfilt.filter_orms(trace,tlength);
                    break;
                default:
                    break;
                }
            }


    //           clipping the traces
             for(int j=0;j<tlength;j++){
             if (grp_clip!=0){
              if(trace[j]>grp_clip){trace[j]=grp_clip;}
              else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
             }
             }



             for(int j=0; j<trace.size(); j++){
                 if(trace[j]>0 && trace[j+1]<0){
                  traceposi[j]=0;
                  double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                  timeposi[j]=yp;
                 }
                 else if(trace[j-1] < 0 && trace[j] > 0){
                   traceposi[j]=0;
                   double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                   timeposi[j]=yp;
                 }
                 else if(trace[j] > 0 && trace[j+1] > 0){
                  traceposi[j]=0;
                 }
                 else{
                     traceposi[j]=trace[j];
                 }
             }

            for(int j=0; j<trace.size(); j++){
                trace[j]=trace[j]+l*1.000005;
                traceposi[j]=traceposi[j]+l*1.000005;
            }






            // This section plots the trace data
            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
            ui->tracePlot->graph(2*l)->setPen(pen);

            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
            ui->tracePlot->graph(2*l+1)->setPen(pennone);
            ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

            ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
            ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


            connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
            ui->tracePlot->yAxis->setRangeReversed(true);

        }// end 3-byte two's complement integer
        break;
    case 8:
    {   // start 1-byte two's complement integer
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }
            QByteArray btrace(f.read(1*tlength));
    //               This loop will make groups of 2 bytes
            int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
            for (quint32 i = 0 ; i < 1*tlength ;i = i + 1) {
            tr_group_temp.append(btrace[i]);


            tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

      //                 exit(0);
            while(tr_group_temp.length()<8) {
             tr_group_temp.insert(0,"0"); // inserting leading zeroes
            }

            trace[j]= -(tr_group_temp.at(0)-48)*pow(2,7);
            for(int k=1; k<8; k++){
                trace[j]+=(tr_group_temp.at(k)-48)*qPow(2,7-k);
            }
    //        qDebug() << fixed << qSetRealNumberPrecision(8) << j << trace[j];
            time[j]=j*intsample*1e-3;//just adding the time vector
            timeposi[j]=j*intsample*1e-3;

            tr_group_temp=0;
            j=j+1;
            }




            if(grp_filttype!=0){
                filter myfilt;
                switch (grp_filttype) {
                case 1:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 2:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 3:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 4:

                    myfilt.filter_orms(trace,tlength);
                    break;
                default:
                    break;
                }
            }


    //           clipping the traces
             for(int j=0;j<tlength;j++){
             if (grp_clip!=0){
              if(trace[j]>grp_clip){trace[j]=grp_clip;}
              else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
             }
             }



             for(int j=0; j<trace.size(); j++){
                 if(trace[j]>0 && trace[j+1]<0){
                  traceposi[j]=0;
                  double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                  timeposi[j]=yp;
                 }
                 else if(trace[j-1] < 0 && trace[j] > 0){
                   traceposi[j]=0;
                   double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                   timeposi[j]=yp;
                 }
                 else if(trace[j] > 0 && trace[j+1] > 0){
                  traceposi[j]=0;
                 }
                 else{
                     traceposi[j]=trace[j];
                 }
             }

            for(int j=0; j<trace.size(); j++){
                trace[j]=trace[j]+l*1.000005;
                traceposi[j]=traceposi[j]+l*1.000005;
            }






            // This section plots the trace data
            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
            ui->tracePlot->graph(2*l)->setPen(pen);

            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
            ui->tracePlot->graph(2*l+1)->setPen(pennone);
            ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

            ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
            ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


            connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
            ui->tracePlot->yAxis->setRangeReversed(true);

        } // end 1-byte two's complement integer
        break;
    case 9:
    {   // start 8-byte two's complement integer
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }
            QByteArray btrace(f.read(8*tlength));
    //               This loop will make groups of 2 bytes
            int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
            for (quint32 i = 0 ; i < 8*tlength ;i = i + 8) {
            tr_group_temp.append(btrace[i]);
            tr_group_temp.append(btrace[i+1]);
            tr_group_temp.append(btrace[i+2]);
            tr_group_temp.append(btrace[i+3]);
            tr_group_temp.append(btrace[i+4]);
            tr_group_temp.append(btrace[i+5]);
            tr_group_temp.append(btrace[i+6]);
            tr_group_temp.append(btrace[i+7]);


            tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

      //                 exit(0);
            while(tr_group_temp.length()<64) {
             tr_group_temp.insert(0,"0"); // inserting leading zeroes
            }

            trace[j]= -(tr_group_temp.at(0)-48)*pow(2,63);
            for(int k=1; k<64; k++){
                trace[j]+=(tr_group_temp.at(k)-48)*qPow(2,63-k);
            }
    //        qDebug() << fixed << qSetRealNumberPrecision(8) << j << trace[j];
            time[j]=j*intsample*1e-3;//just adding the time vector
            timeposi[j]=j*intsample*1e-3;

            tr_group_temp=0;
            j=j+1;
            }




            if(grp_filttype!=0){
                filter myfilt;
                switch (grp_filttype) {
                case 1:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 2:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 3:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 4:

                    myfilt.filter_orms(trace,tlength);
                    break;
                default:
                    break;
                }
            }


    //           clipping the traces
             for(int j=0;j<tlength;j++){
             if (grp_clip!=0){
              if(trace[j]>grp_clip){trace[j]=grp_clip;}
              else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
             }
             }



             for(int j=0; j<trace.size(); j++){
                 if(trace[j]>0 && trace[j+1]<0){
                  traceposi[j]=0;
                  double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                  timeposi[j]=yp;
                 }
                 else if(trace[j-1] < 0 && trace[j] > 0){
                   traceposi[j]=0;
                   double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                   timeposi[j]=yp;
                 }
                 else if(trace[j] > 0 && trace[j+1] > 0){
                  traceposi[j]=0;
                 }
                 else{
                     traceposi[j]=trace[j];
                 }
             }

            for(int j=0; j<trace.size(); j++){
                trace[j]=trace[j]+l*1.000005;
                traceposi[j]=traceposi[j]+l*1.000005;
            }






            // This section plots the trace data
            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
            ui->tracePlot->graph(2*l)->setPen(pen);

            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
            ui->tracePlot->graph(2*l+1)->setPen(pennone);
            ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

            ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
            ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


            connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
            ui->tracePlot->yAxis->setRangeReversed(true);

        }  // end 8-byte two's complement integer
        break;
    case 10:
    {   // start 4-byte unsigned integer
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }
            QByteArray btrace(f.read(4*tlength));
    //               This loop will make groups of 2 bytes
            int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
            for (quint32 i = 0 ; i < 4*tlength ;i = i + 4) {
            tr_group_temp.append(btrace[i]);
            tr_group_temp.append(btrace[i+1]);
            tr_group_temp.append(btrace[i+2]);
            tr_group_temp.append(btrace[i+3]);
            tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

      //                 exit(0);
            while(tr_group_temp.length()<32) {
             tr_group_temp.insert(0,"0"); // inserting leading zeroes
            }
            for(int k=0; k<32; k++){
                trace[j]+=(tr_group_temp.at(k)-48)*qPow(2,31-k);
            }
    //        qDebug() << fixed << qSetRealNumberPrecision(8) << j << tr_group_temp << trace[j];
            time[j]=j*intsample*1e-3;//just adding the time vector
            timeposi[j]=j*intsample*1e-3;

            tr_group_temp=0;
            j=j+1;
            }




            if(grp_filttype!=0){
                filter myfilt;
                switch (grp_filttype) {
                case 1:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 2:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 3:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 4:

                    myfilt.filter_orms(trace,tlength);
                    break;
                default:
                    break;
                }
            }


    //           clipping the traces
             for(int j=0;j<tlength;j++){
             if (grp_clip!=0){
              if(trace[j]>grp_clip){trace[j]=grp_clip;}
              else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
             }
             }



             for(int j=0; j<trace.size(); j++){
                 if(trace[j]>0 && trace[j+1]<0){
                  traceposi[j]=0;
                  double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                  timeposi[j]=yp;
                 }
                 else if(trace[j-1] < 0 && trace[j] > 0){
                   traceposi[j]=0;
                   double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                   timeposi[j]=yp;
                 }
                 else if(trace[j] > 0 && trace[j+1] > 0){
                  traceposi[j]=0;
                 }
                 else{
                     traceposi[j]=trace[j];
                 }
             }

            for(int j=0; j<trace.size(); j++){
                trace[j]=trace[j]+l*1.000005;
                traceposi[j]=traceposi[j]+l*1.000005;
            }






            // This section plots the trace data
            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
            ui->tracePlot->graph(2*l)->setPen(pen);

            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
            ui->tracePlot->graph(2*l+1)->setPen(pennone);
            ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

            ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
            ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


            connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
            ui->tracePlot->yAxis->setRangeReversed(true);
        } // end 4-byte unsigned integer
        break;
    case 11:
    {   // start 2-byte unsigned integer
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }
            QByteArray btrace(f.read(2*tlength));
    //               This loop will make groups of 2 bytes
            int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
            for (quint32 i = 0 ; i < 2*tlength ;i = i + 2) {
            tr_group_temp.append(btrace[i]);
            tr_group_temp.append(btrace[i+1]);
            tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

      //                 exit(0);
            while(tr_group_temp.length()<16) {
             tr_group_temp.insert(0,"0"); // inserting leading zeroes
            }
            for(int k=0; k<16; k++){
                trace[j]+=(tr_group_temp.at(k)-48)*qPow(2,15-k);
            }
    //        qDebug() << fixed << qSetRealNumberPrecision(8) << j << tr_group_temp << trace[j];
            time[j]=j*intsample*1e-3;//just adding the time vector
            timeposi[j]=j*intsample*1e-3;

            tr_group_temp=0;
            j=j+1;
            }




            if(grp_filttype!=0){
                filter myfilt;
                switch (grp_filttype) {
                case 1:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 2:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 3:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 4:

                    myfilt.filter_orms(trace,tlength);
                    break;
                default:
                    break;
                }
            }


    //           clipping the traces
             for(int j=0;j<tlength;j++){
             if (grp_clip!=0){
              if(trace[j]>grp_clip){trace[j]=grp_clip;}
              else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
             }
             }



             for(int j=0; j<trace.size(); j++){
                 if(trace[j]>0 && trace[j+1]<0){
                  traceposi[j]=0;
                  double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                  timeposi[j]=yp;
                 }
                 else if(trace[j-1] < 0 && trace[j] > 0){
                   traceposi[j]=0;
                   double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                   timeposi[j]=yp;
                 }
                 else if(trace[j] > 0 && trace[j+1] > 0){
                  traceposi[j]=0;
                 }
                 else{
                     traceposi[j]=trace[j];
                 }
             }

            for(int j=0; j<trace.size(); j++){
                trace[j]=trace[j]+l*1.000005;
                traceposi[j]=traceposi[j]+l*1.000005;
            }






            // This section plots the trace data
            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
            ui->tracePlot->graph(2*l)->setPen(pen);

            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
            ui->tracePlot->graph(2*l+1)->setPen(pennone);
            ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

            ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
            ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


            connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
            ui->tracePlot->yAxis->setRangeReversed(true);
        } // end 2-byte unsigned integer
        break;
    case 12:
    {   // start 8-byte unsigned integer
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }
            QByteArray btrace(f.read(8*tlength));
    //               This loop will make groups of 2 bytes
            int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
            for (quint32 i = 0 ; i < 8*tlength ;i = i + 8) {
            tr_group_temp.append(btrace[i]);
            tr_group_temp.append(btrace[i+1]);
            tr_group_temp.append(btrace[i+2]);
            tr_group_temp.append(btrace[i+3]);
            tr_group_temp.append(btrace[i+4]);
            tr_group_temp.append(btrace[i+5]);
            tr_group_temp.append(btrace[i+6]);
            tr_group_temp.append(btrace[i+7]);
            tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

      //                 exit(0);
            while(tr_group_temp.length()<64) {
             tr_group_temp.insert(0,"0"); // inserting leading zeroes
            }
            for(int k=0; k<64; k++){
                trace[j]+=(tr_group_temp.at(k)-48)*qPow(2,63-k);
            }
    //        qDebug() << fixed << qSetRealNumberPrecision(8) << j << tr_group_temp << trace[j];
            time[j]=j*intsample*1e-3;//just adding the time vector
            timeposi[j]=j*intsample*1e-3;

            tr_group_temp=0;
            j=j+1;
            }




            if(grp_filttype!=0){
                filter myfilt;
                switch (grp_filttype) {
                case 1:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 2:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 3:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 4:

                    myfilt.filter_orms(trace,tlength);
                    break;
                default:
                    break;
                }
            }


    //           clipping the traces
             for(int j=0;j<tlength;j++){
             if (grp_clip!=0){
              if(trace[j]>grp_clip){trace[j]=grp_clip;}
              else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
             }
             }



             for(int j=0; j<trace.size(); j++){
                 if(trace[j]>0 && trace[j+1]<0){
                  traceposi[j]=0;
                  double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                  timeposi[j]=yp;
                 }
                 else if(trace[j-1] < 0 && trace[j] > 0){
                   traceposi[j]=0;
                   double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                   timeposi[j]=yp;
                 }
                 else if(trace[j] > 0 && trace[j+1] > 0){
                  traceposi[j]=0;
                 }
                 else{
                     traceposi[j]=trace[j];
                 }
             }

            for(int j=0; j<trace.size(); j++){
                trace[j]=trace[j]+l*1.000005;
                traceposi[j]=traceposi[j]+l*1.000005;
            }






            // This section plots the trace data
            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
            ui->tracePlot->graph(2*l)->setPen(pen);

            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
            ui->tracePlot->graph(2*l+1)->setPen(pennone);
            ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

            ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
            ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


            connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
            ui->tracePlot->yAxis->setRangeReversed(true);

        } // end 8-byte unsigned integer
        break;
    case 15:
    {   // start 3-byte unsigned integer
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }
            QByteArray btrace(f.read(3*tlength));
    //               This loop will make groups of 2 bytes
            int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
            for (quint32 i = 0 ; i < 3*tlength ;i = i + 3) {
            tr_group_temp.append(btrace[i]);
            tr_group_temp.append(btrace[i+1]);
            tr_group_temp.append(btrace[i+2]);
            tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

      //                 exit(0);
            while(tr_group_temp.length()<24) {
             tr_group_temp.insert(0,"0"); // inserting leading zeroes
            }
            for(int k=0; k<24; k++){
                trace[j]+=(tr_group_temp.at(k)-48)*qPow(2,23-k);
            }
    //        qDebug() << fixed << qSetRealNumberPrecision(8) << j << tr_group_temp << trace[j];
            time[j]=j*intsample*1e-3;//just adding the time vector
            timeposi[j]=j*intsample*1e-3;

            tr_group_temp=0;
            j=j+1;
            }




            if(grp_filttype!=0){
                filter myfilt;
                switch (grp_filttype) {
                case 1:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 2:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 3:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 4:

                    myfilt.filter_orms(trace,tlength);
                    break;
                default:
                    break;
                }
            }


    //           clipping the traces
             for(int j=0;j<tlength;j++){
             if (grp_clip!=0){
              if(trace[j]>grp_clip){trace[j]=grp_clip;}
              else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
             }
             }



             for(int j=0; j<trace.size(); j++){
                 if(trace[j]>0 && trace[j+1]<0){
                  traceposi[j]=0;
                  double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                  timeposi[j]=yp;
                 }
                 else if(trace[j-1] < 0 && trace[j] > 0){
                   traceposi[j]=0;
                   double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                   timeposi[j]=yp;
                 }
                 else if(trace[j] > 0 && trace[j+1] > 0){
                  traceposi[j]=0;
                 }
                 else{
                     traceposi[j]=trace[j];
                 }
             }

            for(int j=0; j<trace.size(); j++){
                trace[j]=trace[j]+l*1.000005;
                traceposi[j]=traceposi[j]+l*1.000005;
            }






            // This section plots the trace data
            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
            ui->tracePlot->graph(2*l)->setPen(pen);

            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
            ui->tracePlot->graph(2*l+1)->setPen(pennone);
            ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

            ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
            ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


            connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
            ui->tracePlot->yAxis->setRangeReversed(true);
        } // end 3-byte unsigned integer
        break;
    case 16:
    {   // start 1-byte unsigned integer
        grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1);
        f.seek(grp_filepos);  //we must seek to the right trace header reading
        QByteArray thead(f.read(240));  //reading the trace header



        // This section calculates the bytes to display the header attributes (4-bytes words)
                for(int k=0;k<69;k++){
                    for(int i=bi_g[k]-1;i<bi_g[k]+bl_g[k]-1;i++){
                       hd_group_temp.append(thead[i]);
                    }
                    tr_group_lbyte[k]=hd_group_temp;
                    hd_group_temp=0;
                }
            QByteArray btrace(f.read(1*tlength));
    //               This loop will make groups of 2 bytes
            int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
            for (quint32 i = 0 ; i < 1*tlength ;i = i + 1) {
            tr_group_temp.append(btrace[i]);
            tr_group_temp=QByteArray::number(tr_group_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

      //                 exit(0);
            while(tr_group_temp.length()<8) {
             tr_group_temp.insert(0,"0"); // inserting leading zeroes
            }
            for(int k=0; k<8; k++){
                trace[j]+=(tr_group_temp.at(k)-48)*qPow(2,7-k);
            }
//            qDebug() << fixed << qSetRealNumberPrecision(8) << j << tr_group_temp << trace[j];
            time[j]=j*intsample*1e-3;//just adding the time vector
            timeposi[j]=j*intsample*1e-3;

            tr_group_temp=0;
            j=j+1;
            }




            if(grp_filttype!=0){
                filter myfilt;
                switch (grp_filttype) {
                case 1:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 2:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 3:

                    myfilt.filter_butt(trace,tlength,grp_filttype);
                    break;
                case 4:

                    myfilt.filter_orms(trace,tlength);
                    break;
                default:
                    break;
                }
            }


    //           clipping the traces
             for(int j=0;j<tlength;j++){
             if (grp_clip!=0){
              if(trace[j]>grp_clip){trace[j]=grp_clip;}
              else if(trace[j]<grp_clip*(-1)){trace[j]=(-1)*grp_clip;}
             }
             }



             for(int j=0; j<trace.size(); j++){
                 if(trace[j]>0 && trace[j+1]<0){
                  traceposi[j]=0;
                  double yp=time[j+1]-trace[j+1]*(time[j+1]-time[j])/(trace[j+1]-trace[j]);
                  timeposi[j]=yp;
                 }
                 else if(trace[j-1] < 0 && trace[j] > 0){
                   traceposi[j]=0;
                   double yp=time[j-1]-trace[j-1]*(time[j]-time[j-1])/(trace[j]-trace[j-1]);
                   timeposi[j]=yp;
                 }
                 else if(trace[j] > 0 && trace[j+1] > 0){
                  traceposi[j]=0;
                 }
                 else{
                     traceposi[j]=trace[j];
                 }
             }

            for(int j=0; j<trace.size(); j++){
                trace[j]=trace[j]+l*1.000005;
                traceposi[j]=traceposi[j]+l*1.000005;
            }






            // This section plots the trace data
            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l)->setData(time,trace); //adding a graph
            ui->tracePlot->graph(2*l)->setPen(pen);

            ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
            ui->tracePlot->graph(2*l+1)->setData(timeposi,traceposi); //adding a graph
            ui->tracePlot->graph(2*l+1)->setPen(pennone);
            ui->tracePlot->graph(2*l+1)->setSelectable(QCP::stNone);

            ui->tracePlot->graph(2*l)->setBrush(QBrush(QColor(0,0,1)));
            ui->tracePlot->graph(2*l)->setChannelFillGraph(ui->tracePlot->graph(2*l+1));


            connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
            ui->tracePlot->yAxis->setRangeReversed(true);
        } // end 1-byte unsigned integer
        break;
    default:
        break;
    }

   }



    ui->tracePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->tracePlot->axisRect(0)->setRangeDrag(Qt::Vertical|Qt::Horizontal);
    ui->tracePlot->axisRect(0)->setRangeZoom(Qt::Vertical|Qt::Horizontal);
    connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
    ui->tracePlot->yAxis->setRangeReversed(true);
    ui->tracePlot->replot();

    fbcalc=false;


}

































void trace_group_ex::onYRangeChanged(const QCPRange &range)
{
    QCPRange boundedRange = range;
    double lowerRangeBound = 0;
    double upperRangeBound = tlength*intsample/(1.e3);


    if(boundedRange.lower < lowerRangeBound || qFuzzyCompare(boundedRange.size(),upperRangeBound-lowerRangeBound)) {  // restrict max zoom in
        boundedRange.lower = lowerRangeBound;
        boundedRange.upper = lowerRangeBound + boundedRange.size();
        if(boundedRange.upper > upperRangeBound){
            boundedRange.upper = upperRangeBound;
            ui->tracePlot->yAxis->setRange(boundedRange);
        }
   }
    else if(boundedRange.upper > upperRangeBound || qFuzzyCompare(boundedRange.size(),upperRangeBound-lowerRangeBound)) {  // restrict max zoom in
        boundedRange.upper = upperRangeBound;
        boundedRange.lower = upperRangeBound - boundedRange.size();
        if(boundedRange.lower < lowerRangeBound){
            boundedRange.lower = lowerRangeBound;
            ui->tracePlot->yAxis->setRange(boundedRange);
        }
   }

    ui->tracePlot->yAxis->setRange(boundedRange);

}


void trace_group_ex::slotMouseMove(QMouseEvent *ev){

    int l = 0;
    long yc = ui->tracePlot->yAxis->pixelToCoord(ev->pos().y());
    double xc = 0.0;
    if(ui->tracePlot->selectedGraphs().count()!=0){
    QString graphname=ui->tracePlot->selectedGraphs().first()->name();
//    QRegExp rx("(-?\\d+(?:[\\.,]\\d+(?:e\\d+)?)?)");
    QRegExp rx("\\d\\d?\\d?\\d?$");
    int pos=rx.indexIn(graphname);
    if(pos>-1){l = (rx.cap(0).toInt()-1)/2;}
    xc = ui->tracePlot->selectedGraphs().first()->data().data()->at(yc*1e3/intsample)->value-l*1.000005;
    }
    ui->xval->setText("value="+QString::number(xc,'f',6));
    ui->yval->setText("time="+QString::number(yc,10));


}




void trace_group_ex::on_params_clicked()
{

    paramsdlg->show();
    paramsdlg->activateWindow();
}





trace_group_ex::~trace_group_ex()
{
    delete ui;
}



void trace_group_ex::on_filter_clicked()
{
    filterdlg->show();
    filterdlg->activateWindow();
}

void trace_group_ex::on_fbpick_clicked()
{
    fbdialog->show();
    fbdialog->activateWindow();
}

void trace_group_ex::on_prof_drw_toggled(bool checked)
{
    qDebug() << "click,click";
    qDebug() << ui->tracePlot->graphCount();
    //    qDebug() << ui->traceplot->graphCount();

    if(ui->tracePlot->graphCount()<=2&&checked==true){ //one trace conditional checking
         QMessageBox warning_trace;
         warning_trace.warning(0,"Warning","Please select 2 or more traces for an accurate slope estimation.");
         ui->prof_drw->toggle();
         }
    else if(ui->tracePlot->graphCount()>2){


        if(checked==true){
         connect(ui->tracePlot,&QCustomPlot::mousePress, this, &trace_group_ex::slotMouseClick);
         connect(ui->tracePlot,&QCustomPlot::mouseDoubleClick, this, &trace_group_ex::slotMouseDoubleClick);
         ui->tracePlot->axisRect(0)->setRangeDrag(0);
         ui->tracePlot->axisRect(0)->setRangeZoom(0);
         ui->params->setEnabled(!checked);

         ui->label->setVisible(true);
         ui->slope_label->setVisible(true);
         ui->trspacingSpinBox->setVisible(true);


        }
        else if (checked==false){
         ui->tracePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
         ui->tracePlot->axisRect(0)->setRangeDrag(Qt::Vertical|Qt::Horizontal);
         ui->tracePlot->axisRect(0)->setRangeZoom(Qt::Vertical|Qt::Horizontal);
         disconnect(ui->tracePlot,&QCustomPlot::mousePress, this, &trace_group_ex::slotMouseClick);
         disconnect(ui->tracePlot,&QCustomPlot::mouseDoubleClick, this, &trace_group_ex::slotMouseDoubleClick);
         int foundIndex = -1;
         for (int i=0; i<ui->tracePlot->graphCount(); ++i)
         {
           if (ui->tracePlot->graph(i)->name() == "slopep1")
           {
             foundIndex = i;
             break;
           }
         }
         ui->tracePlot->removeGraph(foundIndex);
         ui->tracePlot->removeItem(0);
         nmpoint=0;
         ui->tracePlot->replot();
         ui->params->setEnabled(!checked);


         ui->label->setVisible(false);
         ui->slope_label->setVisible(false);
         ui->trspacingSpinBox->setVisible(false);


        }




    } //end of one trace conditional checking

}





void trace_group_ex::slotMouseDoubleClick(QMouseEvent *ev){


        int foundIndex = -1;
        for (int i=0; i<ui->tracePlot->graphCount(); ++i)
        {
          if (ui->tracePlot->graph(i)->name() == "slopep1")
          {
            foundIndex = i;
            break;
          }
        }
        ui->tracePlot->removeGraph(foundIndex);
        ui->tracePlot->removeItem(0);
        ui->tracePlot->replot();
        nmpoint=0;
        ui->prof_drw->toggle();

}

void trace_group_ex::slotMouseClick(QMouseEvent *ev){


    double xc = ui->tracePlot->xAxis->pixelToCoord(ev->pos().x());
    double yc = ui->tracePlot->yAxis->pixelToCoord(ev->pos().y());



    if(nmpoint==0) {
        QPen marker;
        marker.setWidth(1);
        marker.setColor(QColor(0,0,0,255));
        ui->tracePlot->addGraph();
        sp1g = ui->tracePlot->graphCount()-1;
        ui->tracePlot->graph(sp1g)->removeFromLegend();
        ui->tracePlot->graph(sp1g)->setName("slopep1");
        QCPScatterStyle markerstyle(QCPScatterStyle::ssPlus,QColor(0,0,0,255), QColor(0,0,0,255),7);
        ui->tracePlot->graph(sp1g)->setScatterStyle(markerstyle);
        ui->tracePlot->graph(sp1g)->addData(xc,yc);
        ui->tracePlot->removeItem(0);
        ui->tracePlot->replot();
        nmpoint=nmpoint+1;
        xsi=xc;
        ysi=yc;
    }
    else if(nmpoint==1) {

        int sign;
        ui->tracePlot->graph(sp1g)->addData(xc,yc);
//        qDebug()<< "second marker" << nmpoint << ui->traceplot->graphCount();
        xse=xc;
        yse=yc;
        xsm=(xse+xsi)/2.0;
        ysm=(yse+ysi)/2.0;

        qDebug() << "xsi" << "xse" << "xsi-xse" << "floor(xsi-xse)" << "den";
        qDebug() << xsi << xse << xsi-xse << qFloor(xsi-xse) << (qFloor(qAbs(xsi-xse))*ui->trspacingSpinBox->value()+0.00000001);
        qDebug() << "ysi" << "yse" << "ysi-yse";
        qDebug() << ysi << yse << ysi-yse;


        double slope=(yse-ysi)/(qFloor(qAbs(xsi-xse))*ui->trspacingSpinBox->value()+0.00000001);
        double velocity;
        if (xsi-xse>=0) {sign=1;}
        else {sign=-1;}
        slope=slope*sign*1e-3;
        velocity=1./qAbs(slope);
        nmpoint=nmpoint+1;
        QCPItemText *textLabel = new QCPItemText(ui->tracePlot);
//        ui->traceplot->addItem(textLabel);
        QBrush brush(Qt::white,Qt::SolidPattern);
        //add here conditional if for meter of feet display on map
        if(unit==1){
            textLabel->setText(QString::number(velocity,'f',2)+" m/s");
            ui->slope_label->setText("slope="+QString::number(velocity,'f',2)+" m/s");
        }
        else if(unit==2){
            textLabel->setText(QString::number(velocity,'f',2)+" f/s");
            ui->slope_label->setText("slope="+QString::number(velocity,'f',2)+" f/s");
        }
        else{textLabel->setText("Non-valid measurement units");};
//        textLabel->setText(QString::number(dist,'f',2)+" m");
        textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
        textLabel->setPen(QPen(Qt::black)); // show black border around text
        textLabel->setBrush(brush);
        textLabel->position->setCoords(xsm,ysm);
        ui->tracePlot->replot();
    }
    else if(nmpoint==2){
        ui->tracePlot->removeGraph(sp1g);
        ui->tracePlot->removeItem(0);
        ui->tracePlot->replot();
        nmpoint=0;
    }

}
