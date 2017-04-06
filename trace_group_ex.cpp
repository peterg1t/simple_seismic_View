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
#include "paramdialog.h"


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
quint32 grp_filepos;
int grp_sign;
double grp_mantissa;
int grp_exponent;
QByteArray tr_group_temp;
QByteArray hd_group_temp;
QByteArray tr_group_lbyte[100];



// In this subroutine we need to scan a group of traces and show them.


trace_group_ex::trace_group_ex(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::trace_group_ex)
{
    ui->setupUi(this);
    setWindowTitle("Trace Group Explorer");
    QFile f(modelname);
    QDataStream in(&f);
    f.open(QFile::ReadOnly);
    grp_fillen=f.size();
    f.close();
    grp_trpos=1;
    grp_numtr=1;
    grp_gain=1.0;
    grp_clip=0;
    grp_traceread(grp_trpos,grp_numtr,grp_gain,grp_clip);
    ui->tracePlot->rescaleAxes();
    ui->tracePlot->replot();
    connect(ui->tracePlot,&QCustomPlot::mouseMove, this, &trace_group_ex::slotMouseMove);

    paramsdlg = new paramdialog;
    paramsdlg->setModal(true);
    connect(paramsdlg,SIGNAL(applyParams(int,int,double,double)),this,SLOT(grp_traceread(int,int,double,double)));


}







void trace_group_ex::grp_traceread(int grp_trpos,int grp_numtr, double grp_gain, double grp_clip)
{



    if(grp_trpos>(grp_fillen-3200-400)/(240+4*tlength)){
        grp_trpos=(grp_fillen-3200-400)/(240+4*tlength);
    }



    if (grp_numtr + grp_trpos > (grp_fillen-3200-400)/(240+4*tlength)+1) {
        grp_numtr = (grp_fillen-3200-400)/(240+4*tlength)-grp_trpos+1;
    }

//    connect()


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


    for(int l=0;l<grp_numtr;l++){
//    qDebug() << "Reading initial trace conditions" << l << grp_numtr;
    grp_filepos=3200+400+(240+4*tlength)*(grp_trpos+l-1)+240;
    f.seek(grp_filepos);  //we must seek to the right trace header reading

    if (code==1) {  //  start 4-byte IBM floating point
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
                 time[j]=j*intsample*1e-3;//just adding the time vector
                 timeposi[j]=j*intsample*1e-3;

                 tr_group_temp=0;
                 grp_exponent=0;
                 grp_mantissa=0;
                 j=j+1;
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
    else if (code==2){
            QByteArray btrace(f.read(4*tlength));}// 4-byte two's complement integer
    else if (code==3){QByteArray trace(f.read(2*tlength));}  // 2-byte two's complement integer
    else if (code==4){QByteArray trace(f.read(4*tlength));}  // 4-byte fixed-point with gain
    else if (code==5){ // start 4-byte IEEE floating-point
            QByteArray btrace(f.read(4*tlength));



//               This loop will make groups of 4 bytes
           int j =0;
//                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
           for (quint32 i = 0 ; i < 4 ;i = i + 4) {
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
            if(fraction.at(i)=='1')     grp_mantissa += 1.0/(pow(2,i));
           }

           grp_exponent=0;

           for(int i=0; i < tr_group_temp.length()-23; i++) {
               // iterate through the array to calculate the grp_exponent
           grp_exponent += (tr_group_temp.at(i)-48)*pow(2,7-i);
           }


           trace[j]=grp_gain*grp_sign*(1+grp_mantissa)*pow(2,grp_exponent-127);
           time[j]=j*intsample*1e-3;//just adding the time vector
           timeposi[j]=j*intsample*1e-3;//just adding the time vector


           tr_group_temp=0;
           grp_exponent=0;
           grp_mantissa=0;
           j=j+1;
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
    else if (code==8){QByteArray btrace(f.read(tlength));}    // 1-byte two's complement integer









    }



    ui->tracePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->tracePlot->axisRect(0)->setRangeDrag(Qt::Vertical|Qt::Horizontal);
    ui->tracePlot->axisRect(0)->setRangeZoom(Qt::Vertical|Qt::Horizontal);
    connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
    ui->tracePlot->yAxis->setRangeReversed(true);
    ui->tracePlot->replot();


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








trace_group_ex::~trace_group_ex()
{
    delete ui;
}



void trace_group_ex::on_zoomall_clicked()
{
    ui->tracePlot->rescaleAxes();
    ui->tracePlot->replot();
}

void trace_group_ex::on_zoomin_clicked()
{
    ui->tracePlot->yAxis->rescale(true);
    ui->tracePlot->replot();
}

void trace_group_ex::on_zoomout_clicked()
{
    ui->tracePlot->xAxis->rescale(true);
    ui->tracePlot->replot();
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



