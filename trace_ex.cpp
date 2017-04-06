#include "trace_ex.h"
#include "ui_trace_ex.h"
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
#include <QtCharts>
#include <QSplineSeries>
#include <QGraphicsWidget>
#include <hilbert.h>
#include <vector>
#include <qtextstream.h>
#include <wignerville.h>


using std::vector;

//using namespace QtCharts;


extern QString modelname;
quint32 fillen;
extern int intsample;
extern int tlength;
extern int code;
int trpos;
quint32 filepos;
int sign;
double mantissa;
int exponent;
QByteArray tr_temp;
QByteArray hd_temp;
QByteArray tr_lbyte[100];




void trace_ex::traceread(int trpos){
    QFile f(modelname);
    f.open(QFile::ReadOnly);
    filepos=3200+400+(240+4*tlength)*(trpos-1);
    f.seek(filepos);  //we must seek to the right trace header reading
    QByteArray thead(f.read(240));  //reading the trace header  (we don't need to do this now)
    QVector<double> trace(tlength), time(tlength), traceposi(tlength), timeposi(tlength); //Initializing vector with trace and time axis






    if (code==1) {  //  start 4-byte IBM floating point
                  QByteArray btrace(f.read(4*tlength));

//               This loop will make groups of 4 bytes
                 int j =0;
                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
                 tr_temp.append(btrace[i]);
                 tr_temp.append(btrace[i+1]);
                 tr_temp.append(btrace[i+2]);
                 tr_temp.append(btrace[i+3]);
                 tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1
//                 if(trpos=0 && j==0) qDebug() << "Plotting binary for sample j=" << j << tr_temp;
//                 exit(0);
                 while(tr_temp.length()<32) {
                  tr_temp.insert(0,"0"); // inserting leading zeroes
                 }
                 if(tr_temp.at(0)=='1')  sign =-1; else sign=1; // if bit 0 is 1 number is negative
                 tr_temp.remove(0,1); //removing from element 0 with length 1
                 //                                   // remove sign bit



                 QByteArray fraction = tr_temp.right(24);   //get the fractional part
                 mantissa=0;
                 for(int k=0 ; k< 24; k++) {
                     // iterate through the fraction to claculate the mantissa
                  if(fraction.at(k)=='1')     mantissa += 1.0/(pow(2,k));
                 }

                 exponent=0;

                 for(int k=0; k < tr_temp.length()-24; k++) {
                     // iterate through the array to calculate the exponent
                 exponent += (tr_temp.at(k)-48)*pow(2,6-k);
                 }

                 trace[j]=sign*(mantissa/2)*pow(16,exponent-64);
                 time[j]=j*intsample*1e-3;//just adding the time vector
                 timeposi[j]=j*intsample*1e-3;//just adding the time vector
//                 qDebug() << "trace" << trpos << j << time[j] << trace[j] << tlength;

                 tr_temp=0;
                 exponent=0;
                 mantissa=0;
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





                 // This section calculates the bytes to display the header attributes (4-bytes words)
                 j =0;
                 for (int i = 0 ; i < 27 ;i = i + 4) {
                 hd_temp.append(thead[i]);
                 hd_temp.append(thead[i+1]);
                 hd_temp.append(thead[i+2]);
                 hd_temp.append(thead[i+3]);
        //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
                 tr_lbyte[j]=hd_temp;
                 hd_temp=0;
                 j=j+1;
                 }

                 // This section calculates the bytes to display the header attributes (2-bytes words)

                 for (int i = 28 ; i < 35 ;i = i + 2) {
                 hd_temp.append(thead[i]);
                 hd_temp.append(thead[i+1]);
//                 hd_temp.append(thead[i+2]);
//                 hd_temp.append(thead[i+3]);
//                 qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
                 tr_lbyte[j]=hd_temp;
                 hd_temp=0;
                 j=j+1;
                 }


                 // from 36 to 69 they are 4-byte words again

                 for (int i = 36 ; i < 67 ;i = i + 4) {
                 hd_temp.append(thead[i]);
                 hd_temp.append(thead[i+1]);
                 hd_temp.append(thead[i+2]);
                 hd_temp.append(thead[i+3]);
        //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
                 tr_lbyte[j]=hd_temp;
                 hd_temp=0;
                 j=j+1;
                 }



                 // from 69 to 71 they are 2-byte words again (scalars)
                 for (int i = 68 ; i < 71 ;i = i + 2) {
                 hd_temp.append(thead[i]);
                 hd_temp.append(thead[i+1]);
//                 hd_temp.append(thead[i+2]);
//                 hd_temp.append(thead[i+3]);
//                 QDataStream  <--
//                 qDebug() << "Bytes after EBCDIC"  << static_cast<int16_t>(hd_temp.toHex().toInt(nullptr,16)) << i ;
                 tr_lbyte[j]=hd_temp;
                 hd_temp=0;
                 j=j+1;
                 }



                 // from 72 to 89 they are 4-byte words again

                 for (int i = 72 ; i < 87 ;i = i + 4) {
                 hd_temp.append(thead[i]);
                 hd_temp.append(thead[i+1]);
                 hd_temp.append(thead[i+2]);
                 hd_temp.append(thead[i+3]);
        //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
                 tr_lbyte[j]=hd_temp;
                 hd_temp=0;
                 j=j+1;
                 }


                 // from 89 to 144 they are 2-byte words again

                 for (int i = 88 ; i < 143 ;i = i + 2) {
                 hd_temp.append(thead[i]);
                 hd_temp.append(thead[i+1]);
//                 hd_temp.append(thead[i+2]);
//                 hd_temp.append(thead[i+3]);
        //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
                 tr_lbyte[j]=hd_temp;
                 hd_temp=0;
                 j=j+1;
                 }



                 // from 148 to 179 they are 2-byte words again

                 for (int i = 148 ; i < 179 ;i = i + 2) {
                 hd_temp.append(thead[i]);
                 hd_temp.append(thead[i+1]);
//                 hd_temp.append(thead[i+2]);
//                 hd_temp.append(thead[i+3]);
        //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
                 tr_lbyte[j]=hd_temp;
                 hd_temp=0;
                 j=j+1;
                 }




                 // This section display the trace header on the table

                 QStringList tr_headers;
                 tr_headers << "values" << "description" <<  "bytes";
                 QStringList tr_descr;
                 tr_descr  << "Trace sequence number withing line" << "Trace sequence number withing line" << "FFID" << "Trace number within field record" << "SP" << "CDP ensemble number" << "Trace number" << "Trace identification code" << "Number of vertically summed traces" << "Number of horizontally stacked traces" << "Data use (1-prod, 2-test)" << "Distance from source point to receiver group" << "Receiver group elevation" << "Surface elevation at source" << "Source depth below surface (+ num)" << "Datum elevation at receiver group" << "Datum elevation at source" << "Water depth at source" << "Water depth at group" << "Elevations and depths scalar" << "Coordinates scalar" << "Source coordinate - X" << "Source coordinate - Y" << "Group coordinate - X" << "Group coordinate - Y" << "Coordinate units" << "Weathering velocity" << "Subweathering velocity" << "Uphole time at source (ms)" << "Uphole time at group (ms)" << "Source static corection (ms)" << "Group static correction (ms)"<< "Total static (ms)" << "Lag time A" << "Lag time B" << "Delay recording time (ms)" << "Mute time start (ms)" << "Mute time end (ms)" << "Number of samples in this trace" << "Sample interval for this trace (micro-s)" << "Gain type of field instruments" << "Instrument gain" << "Instrument gain constant" << "Corellated" << "Sweep frequency at start" << "Sweep frequency at end" << "Sweep lentgth (ms)" << "Sweep type" << "Sweep trace taper length at start (ms)" << "Sweep trace taper length at end" << "Taper type" << "Alias filter frequency" << "Alias filter slope" << "Low cut frequency" << "High cut frequency" << "Low cut slope" << "High cut slope" << "Year data recorded" << "Day of year" << "Hour of day" << "Minute of hour" << "Second of minute" << "Time basis code" << "Trace weighting factor" << "Geophone group number of roll switch position 1" << "Geophone group number of trace number 1 within original field record" << "Geophone group number of last trace" << "Gap size" << "Overtravel";
                 QStringList tr_bytes_desc;
                 tr_bytes_desc << "1-4" << "5-8" << "9-12" << "13-16" << "17-20" << "21-24" << "25-28" << "29-30" << "31-32" << "33-34" << "35-36" << "37-40" << "41-44" << "45-48" << "49-52" << "53-56" << "57-60" << "61-64" << "65-68" << "69-70" << "71-72" << "73-76" << "77-80" << "81-84" << "85-88" << "89-90" << "91-92" << "93-94" << "95-96" << "97-98" << "99-100" << "101-102" << "103-104" << "105-106" << "107-108" << "109-110" << "111-112" << "113-114" << "115-116" << "117-118" << "119-120" << "121-122" << "123-124" << "125-126" << "127-128" << "129-130" << "131-132" << "133-134" << "135-136" << "137-138" << "139-140" << "141-142" << "143-144" << "149-150" << "151-152" << "153-154" << "155-156" << "157-158" << "159-160" << "161-162" << "163-164" << "165-166" << "167-168" << "169-170" << "171-172" << "173-174" << "175-176" << "177-178" << "179-180";

                 ui->tableWidgettr->setColumnCount(3);
                 ui->tableWidgettr->setRowCount(69);

                 for (int i=0; i<=25; i++) {
                 if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
                 else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
                 ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
                 ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
                 ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_desc[i]));

                 }

                 for (int i=26; i<69; i++) {
                 ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
                 ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
                 ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
                 ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_desc[i]));
                 }

                 ui->tableWidgettr->setColumnWidth(1,395);
                 ui->tableWidgettr->setColumnWidth(2,50);
                 ui->tableWidgettr->setAlternatingRowColors(true);





// This section plots the trace data-------------------------------
                 ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
                 QPen pen;
                 pen.setStyle(Qt::SolidLine);
                 pen.setWidth(1);
                 pen.setColor(QColor(1,1,1));
                 ui->tracePlot->graph(0)->setName("Trace"+QString::number(trpos));
                 ui->tracePlot->graph(0)->setData(time,trace); //adding a graph
                 ui->tracePlot->xAxis->setLabel("Amplitude"); //adding labels
                 ui->tracePlot->yAxis->setLabel("Time (ms)");
                 ui->tracePlot->graph(0)->setPen(pen);
                 ui->tracePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
                 ui->tracePlot->axisRect(0)->setRangeDrag(Qt::Vertical);
                 ui->tracePlot->axisRect(0)->setRangeZoom(Qt::Vertical);



////                 // This section plots the trace data adding the blank trace to achieve the wiggle effect
                 ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
                 QPen pennone;
                 pennone.setStyle(Qt::NoPen);
//                 pennone.setStyle(Qt::SolidLine);
                 pennone.setWidth(1);
                 pennone.setColor(QColor(1,255,255,255));
                 ui->tracePlot->graph(1)->setName("Trace"+QString::number(trpos));
                 ui->tracePlot->graph(1)->setData(timeposi,traceposi); //adding a graph
                 ui->tracePlot->graph(1)->setPen(pennone);

                 ui->tracePlot->graph(0)->setBrush(QBrush(QColor(0,0,1)));
                 ui->tracePlot->graph(0)->setChannelFillGraph(ui->tracePlot->graph(1));


                 connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
                 ui->tracePlot->yAxis->setRangeReversed(true);
                 ui->tracePlot->rescaleAxes();
                 ui->tracePlot->replot();
// This section plots the trace data-----------END-----------------




                int N=qNextPowerOfTwo(trace.size());
                std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



                for(int i=0;i<tlength;i++){
                    sig[i]={trace[i],0};
                }


//                freq[0]=-(1/(intsample*1e-6))/2;
                freq[0]=0;
                for(int i=1;i<N/2;i++){
                    freq[i]=freq[i-1]+freqinc;
//                    qDebug()<<freq[i]<<intsample;
                }



                fft_complx myfft;                   // computing the fft
                myfft.fft_fwd(sig,N);
                for(int i=0;i<N/2;i++){    //postive half spectra calculation only half of the spectrum 0 -> Nyquist/2
                 possig[i]= std::real(sig[i])*std::real(sig[i])+std::imag(sig[i])*std::imag(sig[i]);  // Implement in the future dB scaling instead of absolute value of the amplitudes.
                }





                // This section plots the frequency spectrum data
                ui->specPlot->addGraph(ui->specPlot->xAxis,ui->specPlot->yAxis);
                QPen pen2;
                pen2.setStyle(Qt::SolidLine);
                pen2.setWidth(1);
                pen2.setColor(QColor(0,0,1));
                ui->specPlot->graph(0)->setName("Frequency Spectrum"+QString::number(trpos));
                ui->specPlot->graph(0)->setData(freq,possig); //adding a graph
                ui->specPlot->yAxis->setLabel("Amplitude"); //adding labels
                ui->specPlot->xAxis->setLabel("Frequency (Hz)");
                ui->specPlot->graph(0)->setPen(pen2);
                ui->specPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
                ui->specPlot->axisRect(0)->setRangeDrag(Qt::Horizontal);
                ui->specPlot->axisRect(0)->setRangeZoom(Qt::Horizontal);
                connect(ui->specPlot->xAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onspecXRangeChanged(QCPRange)));
                ui->specPlot->rescaleAxes();
//                ui->specPlot->yAxis->setRangeReversed(true);
                ui->specPlot->replot();















//****************************************test signal******************************************************************

//                // we need to input a 2D array to this sections therefore lets build a 2d dimensional vector that is going to be passed to the method
////                QVector2D WVD(tlength,tlength);

//                //Initially we will use an analytic signal to test the calculation of the WVD

////                parameter(tt=0.008,Pi=acos(-1.)) ! tt: tiempo total de la seal, Fs: frecuencia de muestreo
//             std::complex<double> sigh[N]={0,0};
//                double tt=2;  // duration of the chirp
//                double Pi=acos(-1);
//                double Fs=200;
//                double Fn=Fs/2;   // Nyquist frequency = Sampling frequency/2.
////                double f1=1000;
////                double f2=2000;
//                double f1=20;
//                double f2=40;
//                double f3=1600;
//                double f4=1500;

//                double t0=0.0; // initial time
//                double dt=(1./Fs);

//                double delta=.1*Fs;

//                double beta=(f2-f1)/(2*tt);
//                double beta2=(f4-f3)/(2*tt);









//                for(int i=0; i<tt*Fs; i++) {
//                double t=t0+i*dt;
//                 if (i<=delta) {sigh[i]={0.0,0.0};}
//                 if(i>delta && i<=tt*Fs-delta)
////                    sigh[i]=sin(2*Pi*(f1+beta*(t0+(qRound(i-delta))*dt))*(t0+(qRound(i-delta))*dt))+1.5*sin(2*Pi*(f3+beta2*(t0+(qRound(i-delta+delta/2.))*dt))*(t0+(qRound(i-delta+delta/2.))*dt));
//                 sigh[i]=sin(2*Pi*(f1+beta*(t0+(qRound(i-delta))*dt))*(t0+(qRound(i-delta))*dt));
//                 if(i>tt*Fs-delta) {sigh[i]={0.0,0.0};}
//                };


                //array test

//                 std::complex<double> sigtest[8] = {{1,0},{1,0},{1,0},{1,0},{1,0},{1,0},{1,0},{0,0}};
//                 double sigtesta[8] = {1,1,1,1,0,0,0,0};
//                 double sigtestb[8] = {0,0,0,0,0,0,0,0};
//                 double sigtesta1[8] = {1,1,1,1,0,0,0,0};
//                 double sigtestb1[8] = {0,0,0,0,0,0,0,0};
//                 double sigtestc[2*8+1] = {0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0};
//                 std::complex<double> sigtestd[4] = {{0,0},{1,2},{2,3},{4,5}};

//****************************************test signal******************************************************************





//           Here we create sigh wich will contain the analytical signal (size of the array is a fft number)
             std::complex<double> sigh[N]={0,0};
             QVector<double> WV(tlength*N);


             for(int i=0;i<tlength;i++){
                 sigh[i]={trace[i],0};
             }





             hilbert myhilbert;
             myhilbert.hilbert_fwd(sigh,N);


             wignerville mywigner;
             mywigner.wignerville_1(sigh,WV,tlength,N);

//             now we plot the Wigner-Ville transform preparing some graphics for the image

//             ui->wvPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
             ui->wvPlot->axisRect()->setupFullAxesBox(true);
             ui->wvPlot->xAxis->setLabel("Time (ms)");
             ui->wvPlot->yAxis->setLabel("Frequency (Hz)");
             ui->wvPlot->yAxis->setRange(0,*std::max_element(freq.begin(),freq.end()));
             ui->wvPlot->xAxis->setRange(0,*std::max_element(time.begin(),time.end()));
             QCPColorMap *colormap = new QCPColorMap(ui->wvPlot->xAxis,ui->wvPlot->yAxis);
//             ui->wvPlot->axisRect(0)->setRangeZoom(Qt::Vertical);
//             ui->wvPlot->axisRect(0)->setRangeDrag(Qt::Vertical);
             colormap->data()->clear();
             colormap->data()->setSize(tlength,N);
             colormap->data()->setRange(QCPRange(0,*std::max_element(time.begin(),time.end())),QCPRange(0,*std::max_element(freq.begin(),freq.end())));

             for(int t=0;t<tlength;t++){
                 for(int i=0;i<N;i++){
//                     colormap->data()->setCell(t,i,std::real(WV[t*N+i])); // This should be the real value - see papers.
                     colormap->data()->setCell(t,i,std::abs(WV[t*N+i])); //plotting the module of the amplitude instead of the amplitude of the real part looks better on the graph and normalizing the graph from 0 to 1.
                 }
             }

             WV.clear();


             QCPColorScale *colorScale = new QCPColorScale(ui->wvPlot);
             ui->wvPlot->plotLayout()->addElement(0,1,colorScale);
             colorScale->setType(QCPAxis::atRight);
             colormap->setColorScale(colorScale);
             colormap->setGradient(QCPColorGradient::gpJet);
             colormap->rescaleDataRange();
             ui->wvPlot->replot();












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
           for (int i = 0 ; i < 4 ;i = i + 4) {
           tr_temp.append(btrace[i]);
           tr_temp.append(btrace[i+1]);
           tr_temp.append(btrace[i+2]);
           tr_temp.append(btrace[i+3]);
           tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1
           while(tr_temp.length()<32) {
            tr_temp.insert(0,"0"); // inserting leading zeroes
           }
           if(tr_temp.at(0)=='1')  sign =-1; else sign=1; // if bit 0 is 1 number is negative
           tr_temp.remove(0,1); //removing from element 0 with length 1
           //                                   // remove sign bit



           QByteArray fraction = tr_temp.right(23);   //get the fractional part
           mantissa=0;
           for(int i=0 ; i < 23; i++) {
               // iterate through the fraction to claculate the mantissa
            if(fraction.at(i)=='1')     mantissa += 1.0/(pow(2,i));
           }

           exponent=0;

           for(int i=0; i < tr_temp.length()-23; i++) {
               // iterate through the array to calculate the exponent
           exponent += (tr_temp.at(i)-48)*pow(2,7-i);
           }


           trace[j]=sign*(1+mantissa)*pow(2,exponent-127);
           time[j]=j*intsample*1e-3;//just adding the time vector
           timeposi[j]=j*intsample*1e-3;//just adding the time vector

           tr_temp=0;
           exponent=0;
           mantissa=0;
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




           // This section calculates the bytes to display the header attributes (4-bytes words)
           j =0;
           for (int i = 0 ; i < 27 ;i = i + 4) {
           hd_temp.append(thead[i]);
           hd_temp.append(thead[i+1]);
           hd_temp.append(thead[i+2]);
           hd_temp.append(thead[i+3]);
  //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
           tr_lbyte[j]=hd_temp;
           hd_temp=0;
           j=j+1;
           }

           // This section calculates the bytes to display the header attributes (2-bytes words)

           for (int i = 28 ; i < 35 ;i = i + 2) {
           hd_temp.append(thead[i]);
           hd_temp.append(thead[i+1]);
//                 hd_temp.append(thead[i+2]);
//                 hd_temp.append(thead[i+3]);
  //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
           tr_lbyte[j]=hd_temp;
           hd_temp=0;
           j=j+1;
           }


           // from 36 to 69 they are 4-byte words again

           for (int i = 36 ; i < 67 ;i = i + 4) {
           hd_temp.append(thead[i]);
           hd_temp.append(thead[i+1]);
           hd_temp.append(thead[i+2]);
           hd_temp.append(thead[i+3]);
  //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
           tr_lbyte[j]=hd_temp;
           hd_temp=0;
           j=j+1;
           }



           // from 69 to 71 they are 2-byte words again (scalars)

           for (int i = 68 ; i < 71 ;i = i + 2) {
           hd_temp.append(thead[i]);
           hd_temp.append(thead[i+1]);
//                 hd_temp.append(thead[i+2]);
//                 hd_temp.append(thead[i+3]);
  //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
           tr_lbyte[j]=hd_temp;
           hd_temp=0;
           j=j+1;
           }



           // from 72 to 89 they are 4-byte words again

           for (int i = 72 ; i < 87 ;i = i + 4) {
           hd_temp.append(thead[i]);
           hd_temp.append(thead[i+1]);
           hd_temp.append(thead[i+2]);
           hd_temp.append(thead[i+3]);
  //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
           tr_lbyte[j]=hd_temp;
           hd_temp=0;
           j=j+1;
           }


           // from 89 to 144 they are 2-byte words again

           for (int i = 88 ; i < 143 ;i = i + 2) {
           hd_temp.append(thead[i]);
           hd_temp.append(thead[i+1]);
//                 hd_temp.append(thead[i+2]);
//                 hd_temp.append(thead[i+3]);
  //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
           tr_lbyte[j]=hd_temp;
           hd_temp=0;
           j=j+1;
           }



           // from 148 to 179 they are 2-byte words again

           for (int i = 148 ; i < 179 ;i = i + 2) {
           hd_temp.append(thead[i]);
           hd_temp.append(thead[i+1]);
//                 hd_temp.append(thead[i+2]);
//                 hd_temp.append(thead[i+3]);
  //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
           tr_lbyte[j]=hd_temp;
           hd_temp=0;
           j=j+1;
           }








           // This section display the trace header on the table

           QStringList tr_headers;
           tr_headers << "values" << "description" <<  "bytes";
           QStringList tr_descr;
           tr_descr  << "Trace sequence number withing line" << "Trace sequence number withing line" << "FFID" << "Trace number within field record" << "SP" << "CDP ensemble number" << "Trace number" << "Trace identification code" << "Number of vertically summed traces" << "Number of horizontally stacked traces" << "Data use (1-prod, 2-test)" << "Distance from source point to receiver group" << "Receiver group elevation" << "Surface elevation at source" << "Source depth below surface (+ num)" << "Datum elevation at receiver group" << "Datum elevation at source" << "Water depth at source" << "Water depth at group" << "Elevations and depths scalar" << "Coordinates scalar" << "Source coordinate - X" << "Source coordinate - Y" << "Group coordinate - X" << "Group coordinate - Y" << "Coordinate units" << "Weathering velocity" << "Subweathering velocity" << "Uphole time at source (ms)" << "Uphole time at group (ms)" << "Source static corection (ms)" << "Group static correction (ms)"<< "Total static (ms)" << "Lag time A" << "Lag time B" << "Delay recording time (ms)" << "Mute time start (ms)" << "Mute time end (ms)" << "Number of samples in this trace" << "Sample interval for this trace (micro-s)" << "Gain type of field instruments" << "Instrument gain" << "Instrument gain constant" << "Corellated" << "Sweep frequency at start" << "Sweep frequency at end" << "Sweep lentgth (ms)" << "Sweep type" << "Sweep trace taper length at start (ms)" << "Sweep trace taper length at end" << "Taper type" << "Alias filter frequency" << "Alias filter slope" << "Low cut frequency" << "High cut frequency" << "Low cut slope" << "High cut slope" << "Year data recorded" << "Day of year" << "Hour of day" << "Minute of hour" << "Second of minute" << "Time basis code" << "Trace weighting factor" << "Geophone group number of roll switch position 1" << "Geophone group number of trace number 1 within original field record" << "Geophone group number of last trace" << "Gap size" << "Overtravel";
           QStringList tr_bytes_desc;
           tr_bytes_desc << "1-4" << "5-8" << "9-12" << "13-16" << "17-20" << "21-24" << "25-28" << "29-30" << "31-32" << "33-34" << "35-36" << "37-40" << "41-44" << "45-48" << "49-52" << "53-56" << "57-60" << "61-64" << "65-68" << "69-70" << "71-72" << "73-76" << "77-80" << "81-84" << "85-88" << "89-90" << "91-92" << "93-94" << "95-96" << "97-98" << "99-100" << "101-102" << "103-104" << "105-106" << "107-108" << "109-110" << "111-112" << "113-114" << "115-116" << "117-118" << "119-120" << "121-122" << "123-124" << "125-126" << "127-128" << "129-130" << "131-132" << "133-134" << "135-136" << "137-138" << "139-140" << "141-142" << "143-144" << "149-150" << "151-152" << "153-154" << "155-156" << "157-158" << "159-160" << "161-162" << "163-164" << "165-166" << "167-168" << "169-170" << "171-172" << "173-174" << "175-176" << "177-178" << "179-180";

           ui->tableWidgettr->setColumnCount(3);
           ui->tableWidgettr->setRowCount(69);

           for (int i=0; i<=25; i++) {
           if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
           else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
           ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
           ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
           ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_desc[i]));

           }

           for (int i=26; i<69; i++) {
           ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
           ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
           ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
           ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_desc[i]));
           }

           ui->tableWidgettr->setColumnWidth(1,395);
           ui->tableWidgettr->setColumnWidth(2,50);
           ui->tableWidgettr->setAlternatingRowColors(true);





// This section plots the trace data-------------------------------
                 ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
                 QPen pen;
                 pen.setStyle(Qt::SolidLine);
                 pen.setWidth(1);
                 pen.setColor(QColor(1,1,1));
                 ui->tracePlot->graph(0)->setName("Trace"+QString::number(trpos));
                 ui->tracePlot->graph(0)->setData(time,trace); //adding a graph
                 ui->tracePlot->xAxis->setLabel("Amplitude"); //adding labels
                 ui->tracePlot->yAxis->setLabel("Time (ms)");
                 ui->tracePlot->graph(0)->setPen(pen);
                 ui->tracePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
                 ui->tracePlot->axisRect(0)->setRangeDrag(Qt::Vertical);
                 ui->tracePlot->axisRect(0)->setRangeZoom(Qt::Vertical);



////                 // This section plots the trace data adding the blank trace to achieve the wiggle effect
                 ui->tracePlot->addGraph(ui->tracePlot->yAxis,ui->tracePlot->xAxis);
                 QPen pennone;
                 pennone.setStyle(Qt::NoPen);
//                 pennone.setStyle(Qt::SolidLine);
                 pennone.setWidth(1);
                 pennone.setColor(QColor(1,255,255,255));
                 ui->tracePlot->graph(1)->setName("Trace"+QString::number(trpos));
                 ui->tracePlot->graph(1)->setData(timeposi,traceposi); //adding a graph
                 ui->tracePlot->graph(1)->setPen(pennone);

                 ui->tracePlot->graph(0)->setBrush(QBrush(QColor(0,0,1)));
                 ui->tracePlot->graph(0)->setChannelFillGraph(ui->tracePlot->graph(1));


                 connect(ui->tracePlot->yAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onYRangeChanged(QCPRange)));
                 ui->tracePlot->yAxis->setRangeReversed(true);
                 ui->tracePlot->rescaleAxes();
                 ui->tracePlot->replot();
// This section plots the trace data-----------END-----------------





           int N=qNextPowerOfTwo(trace.size());
           std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
           QVector<double> freq(N/2-1), possig(N/2-1);  // frequency array for display
           double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



           for(int i=0;i<tlength;i++){
               sig[i]={trace[i],0};
           }


//                freq[0]=-(1/(intsample*1e-6))/2;
           freq[0]=0;
           for(int i=1;i<N/2-1;i++){
               freq[i]=freq[i-1]+freqinc;
           }


           fft_complx myfft;                   // computing the fft
           myfft.fft_fwd(sig,N);
           for(int i=0;i<N/2-1;i++){    //postive half spectra calculation only half of the spectrum 0 -> Nyquist/2
            possig[i]= std::real(sig[i])*std::real(sig[i])+std::imag(sig[i])*std::imag(sig[i]);  // Implement in the future dB scaling instead of absolute value of the amplitudes.
           }




           // This section plots the frequency spectrum data
           ui->specPlot->addGraph(ui->specPlot->xAxis,ui->specPlot->yAxis);
           QPen pen2;
           pen2.setStyle(Qt::SolidLine);
           pen2.setWidth(1);
           pen2.setColor(QColor(0,0,1));
           ui->specPlot->graph(0)->setName("Frequency Spectrum"+QString::number(trpos));
           ui->specPlot->graph(0)->setData(freq,possig); //adding a graph
           ui->specPlot->yAxis->setLabel("Amplitude"); //adding labels
           ui->specPlot->xAxis->setLabel("Frequency (Hz)");
           ui->specPlot->graph(0)->setPen(pen2);
           ui->specPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
           ui->specPlot->axisRect(0)->setRangeDrag(Qt::Horizontal);
           ui->specPlot->axisRect(0)->setRangeZoom(Qt::Horizontal);
           connect(ui->specPlot->xAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onspecXRangeChanged(QCPRange)));
           ui->specPlot->rescaleAxes();
//                ui->specPlot->yAxis->setRangeReversed(true);
           ui->specPlot->replot();



//           Here we create sigh wich will contain the analytical signal (size of the array is a fft number)
             std::complex<double> sigh[N]={0,0};
             QVector<double> WV(tlength*N);


             for(int i=0;i<tlength;i++){
              sigh[i]={trace[i],0};
             }





             hilbert myhilbert;
             myhilbert.hilbert_fwd(sigh,N);


             wignerville mywigner;
             mywigner.wignerville_1(sigh,WV,tlength,N);

//             now we plot the Wigner-Ville transform preparing some graphics for the image

//             ui->wvPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
             ui->wvPlot->axisRect()->setupFullAxesBox(true);
             ui->wvPlot->xAxis->setLabel("Time (ms)");
             ui->wvPlot->yAxis->setLabel("Frequency (Hz)");
             ui->wvPlot->yAxis->setRange(0,*std::max_element(freq.begin(),freq.end()));
             ui->wvPlot->xAxis->setRange(0,*std::max_element(time.begin(),time.end()));
             QCPColorMap *colormap = new QCPColorMap(ui->wvPlot->xAxis,ui->wvPlot->yAxis);
//             ui->wvPlot->axisRect(0)->setRangeZoom(Qt::Vertical);
//             ui->wvPlot->axisRect(0)->setRangeDrag(Qt::Vertical);
             colormap->data()->clear();
             colormap->data()->setSize(tlength,N);
             colormap->data()->setRange(QCPRange(0,*std::max_element(time.begin(),time.end())),QCPRange(0,*std::max_element(freq.begin(),freq.end())));

             for(int t=0;t<tlength;t++){
                 for(int i=0;i<N;i++){
//                     colormap->data()->setCell(t,i,std::real(WV[t*N+i])); // This should be the real value - see papers.
                     colormap->data()->setCell(t,i,std::abs(WV[t*N+i])); //plotting the module of the amplitude instead of the amplitude of the real part looks better on the graph and normalizing the graph from 0 to 1.
                 }
             }

             WV.clear();


             QCPColorScale *colorScale = new QCPColorScale(ui->wvPlot);
             ui->wvPlot->plotLayout()->addElement(0,1,colorScale);
             colorScale->setType(QCPAxis::atRight);
             colormap->setColorScale(colorScale);
             colormap->setGradient(QCPColorGradient::gpJet);
             colormap->rescaleDataRange();
             ui->wvPlot->replot();



        }  // end 4-byte IEEE floating-point
    else if (code==8){QByteArray btrace(f.read(tlength));}    // 1-byte two's complement integer





};





trace_ex::trace_ex(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::trace_ex)
{
    ui->setupUi(this);
    setWindowTitle("Trace Explorer");
    ui->horizontalSlider->setMinimum(1);
    ui->horizontalSlider->setTracking(false);
    QFile f(modelname);
    QDataStream in(&f);
    f.open(QFile::ReadOnly);
    fillen=f.size();
    qDebug() << fillen << tlength;
    ui->horizontalSlider->setMaximum( (fillen-3200-400)/(240+4*tlength) );
    f.close();
    trpos=ui->horizontalSlider->value();
    connect(ui->horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(traceread(int)));
    connect(ui->tracePlot,&QCustomPlot::mouseMove, this, &trace_ex::slotMouseMove);


}


// Here we perform the calculation of the Wigner-Ville distribution
//void trace_ex::wvilledistrocalc(QVector<double> trace, QVector<double> time, QVector2D<double> WVD){

//}


void trace_ex::onYRangeChanged(const QCPRange &range)
{
    QCPRange boundedRange = range;
    double lowerRangeBound = 0;
    double upperRangeBound = tlength*intsample/(1.e3);


    if(boundedRange.lower < lowerRangeBound || qFuzzyCompare(boundedRange.size(),upperRangeBound-lowerRangeBound)) {  // restrict max zoom in
        boundedRange.lower = lowerRangeBound;
        boundedRange.upper = lowerRangeBound + boundedRange.size();
        if(boundedRange.upper > upperRangeBound){
            boundedRange.upper = upperRangeBound;
            ui->wvPlot->xAxis->setRange(boundedRange);
        }
   }
    else if(boundedRange.upper > upperRangeBound || qFuzzyCompare(boundedRange.size(),upperRangeBound-lowerRangeBound)) {  // restrict max zoom in
        boundedRange.upper = upperRangeBound;
        boundedRange.lower = upperRangeBound - boundedRange.size();
        if(boundedRange.lower < lowerRangeBound){
            boundedRange.lower = lowerRangeBound;
            ui->wvPlot->xAxis->setRange(boundedRange);
        }
   }

    ui->tracePlot->yAxis->setRange(boundedRange);
    ui->wvPlot->xAxis->setRange(boundedRange);
    ui->wvPlot->replot();


}

void trace_ex::onspecXRangeChanged(const QCPRange &range)
{
    QCPRange boundedRange = range;
    double lowerRangeBound = 0;
    double upperRangeBound = 1.e6/(2.0*intsample);

    if(boundedRange.lower < lowerRangeBound || qFuzzyCompare(boundedRange.size(),upperRangeBound-lowerRangeBound)) {  // restrict max zoom in
        boundedRange.lower = lowerRangeBound;
        boundedRange.upper = lowerRangeBound + boundedRange.size();
        if(boundedRange.upper > upperRangeBound){
            boundedRange.upper = upperRangeBound;
            ui->wvPlot->yAxis->setRange(boundedRange);
        }
    }

    else if(boundedRange.upper > upperRangeBound || qFuzzyCompare(boundedRange.size(),upperRangeBound-lowerRangeBound)) {  // restrict max zoom in
        boundedRange.upper = upperRangeBound;
        boundedRange.lower = upperRangeBound - boundedRange.size();
        if(boundedRange.lower < lowerRangeBound){
            boundedRange.lower = lowerRangeBound;
            ui->wvPlot->yAxis->setRange(boundedRange);
        }
   }

    ui->specPlot->xAxis->setRange(boundedRange);
    ui->wvPlot->yAxis->setRange(boundedRange);
    ui->wvPlot->replot();

}



void trace_ex::slotMouseMove(QMouseEvent *ev){

    long yc = ui->tracePlot->yAxis->pixelToCoord(ev->pos().y());
    double xc = ui->tracePlot->graph(0)->data().data()->at(yc*1e3/intsample)->value;
    ui->xval->setText("value="+QString::number(xc,'f',6));
    ui->yval->setText("time="+QString::number(yc,10));


}



trace_ex::~trace_ex()
{
    delete ui;
}










