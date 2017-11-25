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
#include <geom_ex.h>
#include <wignervilledll.h>
#include <QtMath>
#include <fftshift.h>


using std::vector;

//using namespace QtCharts;


extern QString modelname;
extern bool filterswitch;
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
bool smooth=false;
bool rescale=false;
int N; //next power of two
int bi[69]={0};
int bl[69]={0};
QStringList tr_bytes_ini;
QStringList tr_bytes_length;  // These are global variables because we will use them in several routines
bool geomChange;





void trace_ex::traceread(int trpos, bool smooth, bool rescale){
    QFile f(modelname);
    f.open(QFile::ReadOnly);
    filepos=3200+400+(240+4*tlength)*(trpos-1);
    f.seek(filepos);  //we must seek to the right trace header reading
    QByteArray thead(f.read(240));  //reading the trace header  (we don't need to do this now)
    QVector<double> trace(tlength), time(tlength), traceposi(tlength), timeposi(tlength); //Initializing vector with trace and time axis


    // Preparing the table
    QStringList tr_headers;
    tr_headers << "values" << "description" <<  "byte" << "byte length";
    QStringList tr_descr;
    tr_descr  << "Trace sequence number withing line" << "Trace sequence number withing line" << "FFID" << "Trace number within field record" << "SP" << "CDP ensemble number" << "Trace number" << "Trace identification code" << "Number of vertically summed traces" << "Number of horizontally stacked traces" << "Data use (1-prod, 2-test)" << "Distance from source point to receiver group" << "Receiver group elevation" << "Surface elevation at source" << "Source depth below surface (+ num)" << "Datum elevation at receiver group" << "Datum elevation at source" << "Water depth at source" << "Water depth at group" << "Elevations and depths scalar" << "Coordinates scalar" << "Source coordinate - X" << "Source coordinate - Y" << "Group coordinate - X" << "Group coordinate - Y" << "Coordinate units" << "Weathering velocity" << "Subweathering velocity" << "Uphole time at source (ms)" << "Uphole time at group (ms)" << "Source static corection (ms)" << "Group static correction (ms)"<< "Total static (ms)" << "Lag time A" << "Lag time B" << "Delay recording time (ms)" << "Mute time start (ms)" << "Mute time end (ms)" << "Number of samples in this trace" << "Sample interval for this trace (micro-s)" << "Gain type of field instruments" << "Instrument gain" << "Instrument gain constant" << "Corellated" << "Sweep frequency at start" << "Sweep frequency at end" << "Sweep lentgth (ms)" << "Sweep type" << "Sweep trace taper length at start (ms)" << "Sweep trace taper length at end" << "Taper type" << "Alias filter frequency" << "Alias filter slope" << "Low cut frequency" << "High cut frequency" << "Low cut slope" << "High cut slope" << "Year data recorded" << "Day of year" << "Hour of day" << "Minute of hour" << "Second of minute" << "Time basis code" << "Trace weighting factor" << "Geophone group number of roll switch position 1" << "Geophone group number of trace number 1 within original field record" << "Geophone group number of last trace" << "Gap size" << "Overtravel";



    tr_bytes_ini.clear();
    tr_bytes_length.clear();
    for(int i = 0 ; i<69;i++){
        tr_bytes_ini << QString::number(bi[i]);
        tr_bytes_length  << QString::number(bl[i]);
    }

//    exit(0);

    QStringList tr_bytes_desc;
    tr_bytes_desc << "1-4" << "5-8" << "9-12" << "13-16" << "17-20" << "21-24" << "25-28" << "29-30" << "31-32" << "33-34" << "35-36" << "37-40" << "41-44" << "45-48" << "49-52" << "53-56" << "57-60" << "61-64" << "65-68" << "69-70" << "71-72" << "73-76" << "77-80" << "81-84" << "85-88" << "89-90" << "91-92" << "93-94" << "95-96" << "97-98" << "99-100" << "101-102" << "103-104" << "105-106" << "107-108" << "109-110" << "111-112" << "113-114" << "115-116" << "117-118" << "119-120" << "121-122" << "123-124" << "125-126" << "127-128" << "129-130" << "131-132" << "133-134" << "135-136" << "137-138" << "139-140" << "141-142" << "143-144" << "149-150" << "151-152" << "153-154" << "155-156" << "157-158" << "159-160" << "161-162" << "163-164" << "165-166" << "167-168" << "169-170" << "171-172" << "173-174" << "175-176" << "177-178" << "179-180";


    ui->tableWidgettr->setColumnCount(4);
    ui->tableWidgettr->setRowCount(69);
    ui->tableWidgettr->setColumnWidth(1,395);
    ui->tableWidgettr->setColumnWidth(2,50);
    ui->tableWidgettr->setAlternatingRowColors(true);

//    qDebug() << "trace=" << trpos;


    switch (code) {
    case 1:
    {  //  start 4-byte IBM floating point
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
//                 qDebug() << trace[j];

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



                     //replace trace header section read by custom read section
//                     .
//                     .
//                     .
//                     .
//                     .
//                     .

                     // This section calculates the bytes to display the header attributes (4-bytes words)

                     for(int k=0;k<69;k++){
                         for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
                            hd_temp.append(thead[i]);
//                            qDebug()<< i << k << bi[k]-1 << bi[k]+bl[k]-1 << hd_temp ;
                         }
                         tr_lbyte[k]=hd_temp;
//                         qDebug()<< k << hd_temp <<  QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,32))) ;
                         hd_temp=0;
                     }




//                     j =0;
//                     for (int i = 0 ; i < 27 ;i = i + 4) {
//                     hd_temp.append(thead[i]);
//                     hd_temp.append(thead[i+1]);
//                     hd_temp.append(thead[i+2]);
//                     hd_temp.append(thead[i+3]);
//                     qDebug() << "Bytes after EBCDIC" << hd_temp << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(nullptr,16) << i ;
//                     tr_lbyte[j]=hd_temp;
////                     qDebug()<< j << QString::number(static_cast<int16_t>(tr_lbyte[j].toHex().toInt(nullptr,16)));
//                     hd_temp=0;
//                     j=j+1;
//                     }




//                     exit(0);



//                     // This section calculates the bytes to display the header attributes (2-bytes words)

//                     for (int i = 28 ; i < 35 ;i = i + 2) {
//                     hd_temp.append(thead[i]);
//                     hd_temp.append(thead[i+1]);
//    //                 hd_temp.append(thead[i+2]);
//    //                 hd_temp.append(thead[i+3]);
//    //                 qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
//                     tr_lbyte[j]=hd_temp;
//                     hd_temp=0;
//                     j=j+1;
//                     }


//                     // from 36 to 69 they are 4-byte words again

//                     for (int i = 36 ; i < 67 ;i = i + 4) {
//                     hd_temp.append(thead[i]);
//                     hd_temp.append(thead[i+1]);
//                     hd_temp.append(thead[i+2]);
//                     hd_temp.append(thead[i+3]);
//            //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
//                     tr_lbyte[j]=hd_temp;
//                     hd_temp=0;
//                     j=j+1;
//                     }



//                     // from 69 to 71 they are 2-byte words again (scalars)
//                     for (int i = 68 ; i < 71 ;i = i + 2) {
//                     hd_temp.append(thead[i]);
//                     hd_temp.append(thead[i+1]);
//    //                 hd_temp.append(thead[i+2]);
//    //                 hd_temp.append(thead[i+3]);
//    //                 QDataStream  <--
//    //                 qDebug() << "Bytes after EBCDIC"  << static_cast<int16_t>(hd_temp.toHex().toInt(nullptr,16)) << i ;
//                     tr_lbyte[j]=hd_temp;
//                     hd_temp=0;
//                     j=j+1;
//                     }



//                     // from 72 to 89 they are 4-byte words again

//                     for (int i = 72 ; i < 87 ;i = i + 4) {
//                     hd_temp.append(thead[i]);
//                     hd_temp.append(thead[i+1]);
//                     hd_temp.append(thead[i+2]);
//                     hd_temp.append(thead[i+3]);
//            //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(&ok,16) << i ;
//                     tr_lbyte[j]=hd_temp;
//                     hd_temp=0;
//                     j=j+1;
//                     }


//                     // from 89 to 144 they are 2-byte words again

//                     for (int i = 88 ; i < 143 ;i = i + 2) {
//                     hd_temp.append(thead[i]);
//                     hd_temp.append(thead[i+1]);
//    //                 hd_temp.append(thead[i+2]);
//    //                 hd_temp.append(thead[i+3]);
//                     qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(nullptr,16) << i << j;
//                     tr_lbyte[j]=hd_temp;
//                     hd_temp=0;
//                     j=j+1;
//                     }



//                     // from 148 to 179 they are 2-byte words again

//                     for (int i = 148 ; i < 179 ;i = i + 2) {
//                     hd_temp.append(thead[i]);
//                     hd_temp.append(thead[i+1]);
//    //                 hd_temp.append(thead[i+2]);
//    //                 hd_temp.append(thead[i+3]);
//                     qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp) << hd_temp.toHex().toInt(nullptr,16) << i << j ;
//                     tr_lbyte[j]=hd_temp;
//                     hd_temp=0;
//                     j=j+1;
//                     }




//                     // This section display the trace header on the table

//                     for (int i=0; i<=25; i++) {
//                     if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
//                     else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
//                     ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
//                     ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
//                     ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
//                     ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
////                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

//                     }

//                     for (int i=26; i<69; i++) {
//                     ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
//                     ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
//                     ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
//                     ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
//                     ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
////                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

//                     }




                     // This section display the trace header on the table

                     for (int i=0; i<69; i++) {
                     if(bl[i]==2){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
                     else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
                     ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
                     ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
                     ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
                     ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
                     }





    // This section computes the FFT of the signal-------------------------------------

                     N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }






          tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);





//   //****************************************test signal******************************************************************

//   //                // we need to input a 2D array to this sections therefore lets build a 2d dimensional vector that is going to be passed to the method
//   ////                QVector2D WVD(tlength,tlength);
//                 QFile g("data_plot.txt");
//                 g.open(QFile::ReadWrite);
//                 QTextStream stream(&g);

//   //                //Initially we will use an analytic signal to test the calculation of the WVD

//   ////                parameter(tt=0.008,Pi=acos(-1.)) ! tt: tiempo total de la seal, Fs: frecuencia de muestreo
//   //             std::complex<double> sigh[N]={0,0};
//                   double tt=2;  // duration of the chirp
//                   double Pi=acos(-1);
//                   double Fs=200;
//                   double Fn=Fs/2;   // Nyquist frequency = Sampling frequency/2.
//   ////                double f1=1000;
//   ////                double f2=2000;
//                   double f1=20;
//                   double f2=40;
//   //                double f3=1600;
//   //                double f4=1500;

//                   double t0=0.0; // initial time
//                   double dt=(1./Fs);

//                   double delta=.1*Fs;

//                   double beta=(f2-f1)/(2*tt);
//   //                double beta2=(f4-f3)/(2*tt);



//                   double pi = qAcos(-1);

//                   double alf1=40;
//                   double chrate1=20;
//                   double bet1=1;
//                   double fc1=20;
//                   double pha1=0;
//                   double tau1=0.75;

//                   double alf2=40;
//                   double chrate2=60;
//                   double bet2=1;
//                   double fc2=40;
//                   double pha2=0;
//                   double tau2=1.5;





////           Here we create sigh wich will contain the analytical signal (size of the array is a fft number)
//             std::complex<double> sigh[N]={0,0};
////             QVector<double> WV(tlength*N);
//             QVector<double> WV(N*N);


//                   for(int i=0; i<tt*Fs; i++) {
//                   double t=t0+i*dt;
//                    if (i<=delta) {sigh[i]={0.0,0.0};}
//                    if(i>delta && i<=tt*Fs-delta)
//   //                    sigh[i]=sin(2*Pi*(f1+beta*(t0+(qRound(i-delta))*dt))*(t0+(qRound(i-delta))*dt))+1.5*sin(2*Pi*(f3+beta2*(t0+(qRound(i-delta+delta/2.))*dt))*(t0+(qRound(i-delta+delta/2.))*dt));
////                    sigh[i]=sin(2*Pi*(f1+beta*(t0+(qRound(i-delta))*dt))*(t0+(qRound(i-delta))*dt));
//                    sigh[i]= bet1*qExp(-alf1*(t-tau1)*(t-tau1))*cos(2*pi*fc1*(t-tau1)+pha1+chrate1*(t-tau1)*(t-tau1))+ bet2*qExp(-alf2*(t-tau2)*(t-tau2))*cos(2*pi*fc2*(t-tau2)+pha2+chrate2*(t-tau2)*(t-tau2));
//                    if(i>tt*Fs-delta) {sigh[i]={0.0,0.0};}
//                    stream << t << "  " << std::real(sigh[i]) << endl;
//                   };


         //array test

//                    std::complex<double> sigtest[24] = {{11,0},{12,0},{13,0},{14,0},{21,0},{22,0},{23,0},{24,0},{31,0},{32,0},{33,0},{34,0},{41,0},{42,0},{43,0},{44,0},{51,0},{52,0},{53,0},{54,0},{61,0},{62,0},{63,0},{64,0}};
   //                 double sigtesta[8] = {1,1,1,1,0,0,0,0};
   //                 double sigtestb[8] = {0,0,0,0,0,0,0,0};
   //                 double sigtesta1[8] = {1,1,1,1,0,0,0,0};
   //                 double sigtestb1[8] = {0,0,0,0,0,0,0,0};
   //                 double sigtestc[2*8+1] = {0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0};
   //                 std::complex<double> sigtestd[4] = {{0,0},{1,2},{2,3},{4,5}};


//                    fftshift myfftshift;
//                    myfftshift.fftshift_2D(sigtest,4,6);
//                    for(int i=0;i<24;i++){
//                    qDebug() << std::real(sigtest[i]);
//                    }
//                    exit(0);

   //****************************************test signal******************************************************************




                 wvplot(trace,N,freq, time,smooth,rescale);



        }   //  end 4-byte IBM floating point
        break;
    case 2:   //begin 4-byte two's complement integer
    {

            QByteArray btrace(f.read(4*tlength));

    //               This loop will make groups of 4 bytes
           int j =0;
           for (int i = 0 ; i < 4*tlength ;i = i + 4) {
           tr_temp.append(btrace[i]);
           tr_temp.append(btrace[i+1]);
           tr_temp.append(btrace[i+2]);
           tr_temp.append(btrace[i+3]);
    //       qDebug() << j << tr_temp.toHex() <<  QByteArray::number(tr_temp.toHex().toInt(nullptr,16));
           tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

           while(tr_temp.length()<32) {
            tr_temp.insert(0,"0"); // inserting leading zeroes
           }

           if(tr_temp.at(0)=='1')  sign =-1; else sign=1; // if bit 0 is 1 number is negative


           trace[j]= -(tr_temp.at(0)-48)*pow(2,31);
           for(int k=1; k<32; k++){
               trace[j]+=(tr_temp.at(k)-48)*qPow(2,31-k);
           }



           time[j]=j*intsample*1e-3;//just adding the time vector
           timeposi[j]=j*intsample*1e-3;//just adding the time vector

           tr_temp=0;
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
           for(int k=0;k<69;k++){
               for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
                  hd_temp.append(thead[i]);
//                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
               }
               tr_lbyte[k]=hd_temp;
//                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
               hd_temp=0;
           }




           // This section display the trace header on the table

           for (int i=0; i<=25; i++) {
           if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
           else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
           ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
           ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
           ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
           ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

           }

           for (int i=26; i<69; i++) {
           ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
           ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
           ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
           ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
           ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

           }









    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }



    tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);





    wvplot(trace,N,freq, time,smooth,rescale);


    } // end 4-byte two's complement integer
        break;
    case 3:
    {  // 2-byte two's complement integer

            QByteArray btrace(f.read(2*tlength));

    //               This loop will make groups of 4 bytes
           int j =0;
           for (int i = 0 ; i < 2*tlength ;i = i + 2) {
           tr_temp.append(btrace[i]);
           tr_temp.append(btrace[i+1]);
    //       qDebug() << j << tr_temp.toHex() <<  QByteArray::number(tr_temp.toHex().toInt(nullptr,16));
           tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

           while(tr_temp.length()<16) {
            tr_temp.insert(0,"0"); // inserting leading zeroes
           }

//           if(tr_temp.at(0)=='1')  sign =-1; else sign=1; // if bit 0 is 1 number is negative


           trace[j]= -(tr_temp.at(0)-48)*pow(2,15);
           for(int k=1; k<32; k++){
               trace[j]+=(tr_temp.at(k)-48)*qPow(2,15-k);
           }



           time[j]=j*intsample*1e-3;//just adding the time vector
           timeposi[j]=j*intsample*1e-3;//just adding the time vector

           tr_temp=0;
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
           for(int k=0;k<69;k++){
               for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
                  hd_temp.append(thead[i]);
//                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
               }
               tr_lbyte[k]=hd_temp;
//                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
               hd_temp=0;
           }




           // This section display the trace header on the table

           for (int i=0; i<=25; i++) {
           if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
           else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
           ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
           ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
           ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
           ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

           }

           for (int i=26; i<69; i++) {
           ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
           ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
           ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
           ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
           ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

           }









    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }



          tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);

          wvplot(trace,N,freq, time,smooth,rescale);

   } // end 2-byte two's complement integer


        break;
    case 4:

        break;
    case 5:
    { // start 4-byte IEEE floating-point
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
                if(fraction.at(i)=='1')     mantissa += 1.0/(pow(2,i+1));
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
               for(int k=0;k<69;k++){
                   for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
                      hd_temp.append(thead[i]);
    //                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
                   }
                   tr_lbyte[k]=hd_temp;
    //                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
                   hd_temp=0;
               }








               // This section display the trace header on the table

               for (int i=0; i<=25; i++) {
               if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
               else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
               ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
               ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
               ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
               ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

               }

               for (int i=26; i<69; i++) {
               ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
               ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
               ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
               ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
               ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

               }







    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }



    tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);

    wvplot(trace,N,freq, time,smooth,rescale);


            }  // end 4-byte IEEE floating-point
        break;
    case 6:
    { // start 8-byte IEEE floating-point
                QByteArray btrace(f.read(8*tlength));

    //               This loop will make groups of 4 bytes
               int j =0;
    //                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
               for (int i = 0 ; i < 8*tlength ;i = i + 8) {
               tr_temp.append(btrace[i]);
               tr_temp.append(btrace[i+1]);
               tr_temp.append(btrace[i+2]);
               tr_temp.append(btrace[i+3]);
               tr_temp.append(btrace[i+4]);
               tr_temp.append(btrace[i+5]);
               tr_temp.append(btrace[i+6]);
               tr_temp.append(btrace[i+7]);
               tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1
               while(tr_temp.length()<64) {
                tr_temp.insert(0,"0"); // inserting leading zeroes
               }
               if(tr_temp.at(0)=='1')  sign =-1; else sign=1; // if bit 0 is 1 number is negative
               tr_temp.remove(0,1); //removing from element 0 with length 1
               //                                   // remove sign bit



               QByteArray fraction = tr_temp.right(52);   //get the fractional part
               mantissa=0;
               for(int i=0 ; i < 52; i++) {
                   // iterate through the fraction to claculate the mantissa
                if(fraction.at(i)=='1')     mantissa += 1.0/(pow(2,i+1));
               }

               exponent=0;

               for(int i=0; i < tr_temp.length()-52; i++) {
                   // iterate through the array to calculate the exponent
               exponent += (tr_temp.at(i)-48)*pow(2,10-i);
               }


               trace[j]=sign*(1+mantissa)*pow(2,exponent-1023);
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
               for(int k=0;k<69;k++){
                   for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
                      hd_temp.append(thead[i]);
    //                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
                   }
                   tr_lbyte[k]=hd_temp;
    //                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
                   hd_temp=0;
               }






               // This section display the trace header on the table

               for (int i=0; i<=25; i++) {
               if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
               else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
               ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
               ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
               ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
               ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

               }

               for (int i=26; i<69; i++) {
               ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
               ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
               ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
               ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
               ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

               }










    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }



        tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);

        wvplot(trace,N,freq, time,smooth,rescale);



            }  // end 8-byte IEEE floating-point
        break;
    case 7:

    { // 3-byte two's complement integer
        QByteArray btrace(f.read(3*tlength));
//               This loop will make groups of 2 bytes
        int j =0;
//                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
        for (quint32 i = 0 ; i < 3*tlength ;i = i + 3) {
        tr_temp.append(btrace[i]);
        tr_temp.append(btrace[i+1]);
        tr_temp.append(btrace[i+2]);
        tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

  //                 exit(0);
        while(tr_temp.length()<24) {
         tr_temp.insert(0,"0"); // inserting leading zeroes
        }

        trace[j]= -(tr_temp.at(0)-48)*pow(2,23);
        for(int k=1; k<24; k++){
            trace[j]+=(tr_temp.at(k)-48)*qPow(2,23-k);
        }
        time[j]=j*intsample*1e-3;//just adding the time vector
        timeposi[j]=j*intsample*1e-3;

        tr_temp=0;
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
        for(int k=0;k<69;k++){
            for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
               hd_temp.append(thead[i]);
//                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
            }
            tr_lbyte[k]=hd_temp;
//                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
            hd_temp=0;
        }







        // This section display the trace header on the table

        for (int i=0; i<=25; i++) {
        if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
        else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }

        for (int i=26; i<69; i++) {
        ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }









    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }



    tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);

    wvplot(trace,N,freq, time,smooth,rescale);



            }  // end 3-byte two's complement integer
        break;
    case 8:

    { // 1-byte two's complement integer
        QByteArray btrace(f.read(1*tlength));
//               This loop will make groups of 2 bytes
        int j =0;
//                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
        for (quint32 i = 0 ; i < 1*tlength ;i = i + 1) {
        tr_temp.append(btrace[i]);
        tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

  //                 exit(0);
        while(tr_temp.length()<8) {
         tr_temp.insert(0,"0"); // inserting leading zeroes
        }

        trace[j]= -(tr_temp.at(0)-48)*pow(2,7);
        for(int k=1; k<8; k++){
            trace[j]+=(tr_temp.at(k)-48)*qPow(2,7-k);
        }
        time[j]=j*intsample*1e-3;//just adding the time vector
        timeposi[j]=j*intsample*1e-3;

        tr_temp=0;
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
        for(int k=0;k<69;k++){
            for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
               hd_temp.append(thead[i]);
//                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
            }
            tr_lbyte[k]=hd_temp;
//                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
            hd_temp=0;
        }







        // This section display the trace header on the table

        for (int i=0; i<=25; i++) {
        if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
        else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }

        for (int i=26; i<69; i++) {
        ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }







    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }



    tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);

    wvplot(trace,N,freq, time,smooth,rescale);



            }  // end 1-byte two's complement integer
        break;
    case 9:

    { // 8-byte two's complement integer
        QByteArray btrace(f.read(8*tlength));
//               This loop will make groups of 2 bytes
        int j =0;
//                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
        for (quint32 i = 0 ; i < 8*tlength ;i = i + 8) {
        tr_temp.append(btrace[i]);
        tr_temp.append(btrace[i+1]);
        tr_temp.append(btrace[i+2]);
        tr_temp.append(btrace[i+3]);
        tr_temp.append(btrace[i+4]);
        tr_temp.append(btrace[i+5]);
        tr_temp.append(btrace[i+6]);
        tr_temp.append(btrace[i+7]);
        tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

  //                 exit(0);
        while(tr_temp.length()<64) {
         tr_temp.insert(0,"0"); // inserting leading zeroes
        }

        trace[j]= -(tr_temp.at(0)-48)*pow(2,63);
        for(int k=1; k<64; k++){
            trace[j]+=(tr_temp.at(k)-48)*qPow(2,63-k);
        }
        time[j]=j*intsample*1e-3;//just adding the time vector
        timeposi[j]=j*intsample*1e-3;

        tr_temp=0;
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
        for(int k=0;k<69;k++){
            for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
               hd_temp.append(thead[i]);
//                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
            }
            tr_lbyte[k]=hd_temp;
//                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
            hd_temp=0;
        }







        // This section display the trace header on the table

        for (int i=0; i<=25; i++) {
        if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
        else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }

        for (int i=26; i<69; i++) {
        ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }






    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }



    tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);

    wvplot(trace,N,freq, time,smooth,rescale);



            }  // end 8-byte two's complement integer
        break;
    case 10:

    { // start 4-byte unsigned integer
        QByteArray btrace(f.read(4*tlength));
//               This loop will make groups of 2 bytes
        int j =0;
//                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
        for (quint32 i = 0 ; i < 4*tlength ;i = i + 4) {
        tr_temp.append(btrace[i]);
        tr_temp.append(btrace[i+1]);
        tr_temp.append(btrace[i+2]);
        tr_temp.append(btrace[i+3]);

        tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

  //                 exit(0);
        while(tr_temp.length()<32) {
          tr_temp.insert(0,"0"); // inserting leading zeroes
         }
         for(int k=0; k<32; k++){
             trace[j]+=(tr_temp.at(k)-48)*qPow(2,31-k);
         }
        time[j]=j*intsample*1e-3;//just adding the time vector
        timeposi[j]=j*intsample*1e-3;

        tr_temp=0;
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
        for(int k=0;k<69;k++){
            for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
               hd_temp.append(thead[i]);
//                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
            }
            tr_lbyte[k]=hd_temp;
//                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
            hd_temp=0;
        }







        // This section display the trace header on the table

        for (int i=0; i<=25; i++) {
        if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
        else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }

        for (int i=26; i<69; i++) {
        ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }








    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }



    tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);

    wvplot(trace,N,freq, time,smooth,rescale);



            }  // end 4-byte unsigned integer
        break;
    case 11:

    { // start 2-byte unsigned integer
        QByteArray btrace(f.read(2*tlength));
//               This loop will make groups of 2 bytes
        int j =0;
//                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
        for (quint32 i = 0 ; i < 2*tlength ;i = i + 2) {
        tr_temp.append(btrace[i]);
        tr_temp.append(btrace[i+1]);

        tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

  //                 exit(0);
        while(tr_temp.length()<16) {
          tr_temp.insert(0,"0"); // inserting leading zeroes
         }
         for(int k=0; k<16; k++){
             trace[j]+=(tr_temp.at(k)-48)*qPow(2,15-k);
         }
        time[j]=j*intsample*1e-3;//just adding the time vector
        timeposi[j]=j*intsample*1e-3;

        tr_temp=0;
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
        for(int k=0;k<69;k++){
            for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
               hd_temp.append(thead[i]);
//                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
            }
            tr_lbyte[k]=hd_temp;
//                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
            hd_temp=0;
        }








        // This section display the trace header on the table

        for (int i=0; i<=25; i++) {
        if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
        else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }

        for (int i=26; i<69; i++) {
        ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }






    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }



    tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);

    wvplot(trace,N,freq, time,smooth,rescale);



            }  // end 2-byte unsigned integer
        break;
    case 12:

    { // start 8-byte unsigned integer
        QByteArray btrace(f.read(8*tlength));
//               This loop will make groups of 2 bytes
        int j =0;
//                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
        for (quint32 i = 0 ; i < 8*tlength ;i = i + 8) {
        tr_temp.append(btrace[i]);
        tr_temp.append(btrace[i+1]);
        tr_temp.append(btrace[i+2]);
        tr_temp.append(btrace[i+3]);
        tr_temp.append(btrace[i+4]);
        tr_temp.append(btrace[i+5]);
        tr_temp.append(btrace[i+6]);
        tr_temp.append(btrace[i+7]);

        tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

  //                 exit(0);
        while(tr_temp.length()<64) {
          tr_temp.insert(0,"0"); // inserting leading zeroes
         }
         for(int k=0; k<64; k++){
             trace[j]+=(tr_temp.at(k)-48)*qPow(2,63-k);
         }
        time[j]=j*intsample*1e-3;//just adding the time vector
        timeposi[j]=j*intsample*1e-3;

        tr_temp=0;
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
        for(int k=0;k<69;k++){
            for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
               hd_temp.append(thead[i]);
//                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
            }
            tr_lbyte[k]=hd_temp;
//                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
            hd_temp=0;
        }








        // This section display the trace header on the table

        for (int i=0; i<=25; i++) {
        if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
        else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }

        for (int i=26; i<69; i++) {
        ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }






    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }



              tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);



               wvplot(trace,N,freq, time,smooth,rescale);



            }  // end 8-byte unsigned integer
        break;
    case 15:

    { // start 8-byte unsigned integer
        QByteArray btrace(f.read(3*tlength));
//               This loop will make groups of 2 bytes
        int j =0;
//                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
        for (quint32 i = 0 ; i < 3*tlength ;i = i + 3) {
        tr_temp.append(btrace[i]);
        tr_temp.append(btrace[i+1]);
        tr_temp.append(btrace[i+2]);

        tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

  //                 exit(0);
        while(tr_temp.length()<24) {
          tr_temp.insert(0,"0"); // inserting leading zeroes
         }
         for(int k=0; k<24; k++){
             trace[j]+=(tr_temp.at(k)-48)*qPow(2,23-k);
         }
        time[j]=j*intsample*1e-3;//just adding the time vector
        timeposi[j]=j*intsample*1e-3;

        tr_temp=0;
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
        for(int k=0;k<69;k++){
            for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
               hd_temp.append(thead[i]);
//                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
            }
            tr_lbyte[k]=hd_temp;
//                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
            hd_temp=0;
        }








        // This section display the trace header on the table

        for (int i=0; i<=25; i++) {
        if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
        else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }

        for (int i=26; i<69; i++) {
        ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }









    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }


               tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);



               wvplot(trace,N,freq, time,smooth,rescale);



            }  // end 3-byte unsigned integer
        break;
    case 16:

    { // start 8-byte unsigned integer
        QByteArray btrace(f.read(1*tlength));
//               This loop will make groups of 2 bytes
        int j =0;
//                 for (int i = 0 ; i < 4*tlength ;i = i + 4) {
        for (quint32 i = 0 ; i < 1*tlength ;i = i + 1) {
        tr_temp.append(btrace[i]);

        tr_temp=QByteArray::number(tr_temp.toHex().toLongLong(nullptr,16),2); //converting the array to a binary 0,1

  //                 exit(0);
        while(tr_temp.length()<8) {
          tr_temp.insert(0,"0"); // inserting leading zeroes
         }
         for(int k=0; k<8; k++){
             trace[j]+=(tr_temp.at(k)-48)*qPow(2,7-k);
         }
        time[j]=j*intsample*1e-3;//just adding the time vector
        timeposi[j]=j*intsample*1e-3;

        tr_temp=0;
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
        for(int k=0;k<69;k++){
            for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
               hd_temp.append(thead[i]);
//                            qDebug()<< i << bi[k]+bl[k] << hd_temp ;
            }
            tr_lbyte[k]=hd_temp;
//                         qDebug()<< k << QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,16))) ;
            hd_temp=0;
        }








        // This section display the trace header on the table

        for (int i=0; i<=25; i++) {
        if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
        else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }

        for (int i=26; i<69; i++) {
        ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
        ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
        ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
        ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
        ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

        }









    // This section computes the FFT of the signal-------------------------------------

                     int N=qNextPowerOfTwo(trace.size());
                     std::complex<double> sig[N]={0,0}; //temporary storage for the complex data on output it will be put back on the vector
                     QVector<double> freq(N/2), possig(N/2);  // frequency array for display
                     double freqinc=(1/(intsample*1e-6))/N; // increment frequency Nyquist frequency / N



    //                 qDebug()<<"Padded trace";
                     for(int i=0;i<tlength;i++){
                         sig[i]={trace[i],0};
                     }


                     freq[0]=0;
                     for(int i=1;i<N/2;i++){
                         freq[i]=freq[i-1]+freqinc;
                     }




              tracespecplot(trace,N,freq,time,sig,possig,traceposi,timeposi,rescale);

              wvplot(trace,N,freq, time, smooth,rescale);



            }  // end 1-byte unsigned integer
        break;
    default:
        break;
    }


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
//    connect(ui->horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(traceread(int)));
    connect(this,SIGNAL(wvd_signal(int,bool,bool)),this,SLOT(traceread(int,bool,bool)));
    connect(ui->tracePlot,&QCustomPlot::mouseMove, this, &trace_ex::slotMouseMove);

    QCPTextElement *title_trace = new QCPTextElement(ui->tracePlot);
    title_trace->setText("Trace amplitude");
    title_trace->setFont(QFont("sans", 10, QFont::Bold));
    ui->tracePlot->plotLayout()->insertRow(0);
    ui->tracePlot->plotLayout()->addElement(0,0,title_trace);


    QCPTextElement *title_wv = new QCPTextElement(ui->wvPlot);
    title_wv->setText("Wigner-Ville Distribution Function");
    title_wv->setFont(QFont("sans", 10, QFont::Bold));
    ui->wvPlot->plotLayout()->insertRow(0);
    ui->wvPlot->plotLayout()->addElement(0,0,title_wv);


    QCPTextElement *title_spec = new QCPTextElement(ui->specPlot);
    title_spec->setText("Frequency spectrum");
    title_spec->setFont(QFont("sans", 10, QFont::Bold));
    ui->specPlot->plotLayout()->insertRow(0);
    ui->specPlot->plotLayout()->addElement(0,0,title_spec);



    // Preparing the table
    QStringList tr_headers;
    tr_headers << "values" << "description" <<  "byte" << "byte length";
    QStringList tr_descr;
    tr_descr  << "Trace sequence number withing line" << "Trace sequence number withing line" << "FFID" << "Trace number within field record" << "SP" << "CDP ensemble number" << "Trace number" << "Trace identification code" << "Number of vertically summed traces" << "Number of horizontally stacked traces" << "Data use (1-prod, 2-test)" << "Distance from source point to receiver group" << "Receiver group elevation" << "Surface elevation at source" << "Source depth below surface (+ num)" << "Datum elevation at receiver group" << "Datum elevation at source" << "Water depth at source" << "Water depth at group" << "Elevations and depths scalar" << "Coordinates scalar" << "Source coordinate - X" << "Source coordinate - Y" << "Group coordinate - X" << "Group coordinate - Y" << "Coordinate units" << "Weathering velocity" << "Subweathering velocity" << "Uphole time at source (ms)" << "Uphole time at group (ms)" << "Source static corection (ms)" << "Group static correction (ms)"<< "Total static (ms)" << "Lag time A" << "Lag time B" << "Delay recording time (ms)" << "Mute time start (ms)" << "Mute time end (ms)" << "Number of samples in this trace" << "Sample interval for this trace (micro-s)" << "Gain type of field instruments" << "Instrument gain" << "Instrument gain constant" << "Corellated" << "Sweep frequency at start" << "Sweep frequency at end" << "Sweep lentgth (ms)" << "Sweep type" << "Sweep trace taper length at start (ms)" << "Sweep trace taper length at end" << "Taper type" << "Alias filter frequency" << "Alias filter slope" << "Low cut frequency" << "High cut frequency" << "Low cut slope" << "High cut slope" << "Year data recorded" << "Day of year" << "Hour of day" << "Minute of hour" << "Second of minute" << "Time basis code" << "Trace weighting factor" << "Geophone group number of roll switch position 1" << "Geophone group number of trace number 1 within original field record" << "Geophone group number of last trace" << "Gap size" << "Overtravel";


    bi[0]=1;
    bl[0]=4;
    for(int i = 1 ; i<7;i++){
    bi[i]=bi[i-1]+bl[i-1];
    bl[i]=4;
    }
    for(int i = 7 ; i < 11;i++){
    bi[i]=bi[i-1]+bl[i-1];
    bl[i]=2;
    }
    for(int i = 11 ; i < 19;i++){
    bi[i]=bi[i-1]+bl[i-1];
    bl[i]=4;
    }
    for(int i = 19 ; i < 21;i++){
    bi[i]=bi[i-1]+bl[i-1];
    bl[i]=2;
    }
    for(int i = 21 ; i < 25;i++){
    bi[i]=bi[i-1]+bl[i-1];
    bl[i]=4;
    }
    for(int i = 25 ; i < 53;i++){
    bi[i]=bi[i-1]+bl[i-1];
    bl[i]=2;
    }
    bi[53]=149;
    bl[53]=2;
    for(int i = 54 ; i < 69;i++){
    bi[i]=bi[i-1]+bl[i-1];
    bl[i]=2;
    }


    QStringList tr_bytes_ini;
    QStringList tr_bytes_length;
    for(int i = 0 ; i<69;i++){
        tr_bytes_ini << QString::number(bi[i]);
        tr_bytes_length  << QString::number(bl[i]);
    }

    QStringList tr_bytes_desc;
    tr_bytes_desc << "1-4" << "5-8" << "9-12" << "13-16" << "17-20" << "21-24" << "25-28" << "29-30" << "31-32" << "33-34" << "35-36" << "37-40" << "41-44" << "45-48" << "49-52" << "53-56" << "57-60" << "61-64" << "65-68" << "69-70" << "71-72" << "73-76" << "77-80" << "81-84" << "85-88" << "89-90" << "91-92" << "93-94" << "95-96" << "97-98" << "99-100" << "101-102" << "103-104" << "105-106" << "107-108" << "109-110" << "111-112" << "113-114" << "115-116" << "117-118" << "119-120" << "121-122" << "123-124" << "125-126" << "127-128" << "129-130" << "131-132" << "133-134" << "135-136" << "137-138" << "139-140" << "141-142" << "143-144" << "149-150" << "151-152" << "153-154" << "155-156" << "157-158" << "159-160" << "161-162" << "163-164" << "165-166" << "167-168" << "169-170" << "171-172" << "173-174" << "175-176" << "177-178" << "179-180";


    ui->tableWidgettr->setColumnCount(4);
    ui->tableWidgettr->setRowCount(69);
    ui->tableWidgettr->setColumnWidth(1,395);
    ui->tableWidgettr->setColumnWidth(2,50);
    ui->tableWidgettr->setAlternatingRowColors(true);



    //Populating the table on init
    for (int i=0; i<=25; i++) {
    if((i==7)||(i==8)||(i==9)||(i==10)||(i==19)||(i==20)){ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));}
    else{ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(tr_lbyte[i].toHex().toInt(nullptr,16))));}
    ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
    ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
    ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
    ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

    }

    for (int i=26; i<69; i++) {
    ui->tableWidgettr->setItem(i,0, new QTableWidgetItem(QString::number(static_cast<int16_t>(tr_lbyte[i].toHex().toInt(nullptr,16)))));
    ui->tableWidgettr->setHorizontalHeaderLabels(tr_headers);
    ui->tableWidgettr->setItem(i,1, new QTableWidgetItem(tr_descr[i]));
    ui->tableWidgettr->setItem(i,2, new QTableWidgetItem(tr_bytes_ini[i]));
    ui->tableWidgettr->setItem(i,3, new QTableWidgetItem(tr_bytes_length[i]));
//                     ui->tableWidgettr->setItem(i,4, new QTableWidgetItem(tr_bytes_desc[i]));

    }

    connect(this,SIGNAL(reloadTrace(int,bool,bool)),this,SLOT(traceread(int,bool,bool)));

    geomwin = new geom_ex;
    geomwin->show();
    connect(ui->horizontalSlider,SIGNAL(valueChanged(int)),geomwin,SLOT(shotrechighlight(int)));



}




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










void trace_ex::tracespecplot(QVector<double> &trace, int N, QVector<double> &freq, QVector<double> &time, std::complex<double> sig[], QVector<double> &possig, QVector<double> &traceposi, QVector<double> &timeposi, bool rescale){


    fft_complx myfft;                   // computing the fft
    myfft.fft_fwd(sig,N);


    for(int i=0;i<N/2;i++){    //postive half spectra calculation only half of the spectrum 0 -> Nyquist/2
     possig[i]= 1e20*(1/(N*(1/(intsample*1e-6))))*(std::real(sig[i])*std::real(sig[i])+std::imag(sig[i])*std::imag(sig[i]));  // Implement in the future dB scaling instead of absolute value of the amplitudes.
    }

    double refpow  = *std::max_element(std::begin(possig),std::end(possig));
     for(int i=0;i<N/2;i++){    //postive half spectra calculation only half of the spectrum 0 -> Nyquist/2
//                   qDebug() << i << possig[i] << refpow <<  log10((possig[i]+0.000000001)/refpow);
      possig[i]= 10*log10((possig[i]+0.000000001)/refpow);

     }




myfft.fft_bwd(sig,N);

    //                 qDebug()<<"Padded trace";
for(int i=0;i<tlength;i++){
 trace[i]=std::real(sig[i]);
}



// This section computes the FFT of the signal--------END-------------------------



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
      if(rescale==true){ui->tracePlot->rescaleAxes();}
//      ui->tracePlot->rescaleAxes();
      ui->tracePlot->replot();
// This section plots the trace data-----------END-----------------





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
       if(rescale==true){ui->specPlot->rescaleAxes();}
//       ui->specPlot->rescaleAxes();
       ui->specPlot->replot();


}
















void trace_ex::wvplot(QVector<double> &trace, int N, QVector<double> &freq, QVector<double> &time, bool smooth, bool rescale){
//               Here we create sigh wich will contain the analytical signal (size of the array is a fft number)
                     std::complex<double> sigh[N]={0,0};
                     QVector<double> WV(N*N);




                     for(int i=0;i<tlength;i++){
                         sigh[i]={trace[i],0};
                     }



                     hilbert myhilbert;
                     myhilbert.hilbert_fwd(sigh,N);



// Using the built-in wigner routine
//                     wignerville mywigner;
// Using the DLL wigner routine
                     Wignervilledll mywigner;

                     switch (smooth) {
                     case false:
//                     mywigner.wignerville_fwd(sigh,WV,tlength,N);
                     mywigner.wignervilledll_fwd(sigh,WV,tlength,N);
                         break;
                     case true:
//                     mywigner.wignerville_fwd_kernel(sigh,WV,N,N);
                     mywigner.wignervilledll_fwd_kernel(sigh,WV,N,N);
                         break;
                     }




        //             now we plot the Wigner-Ville transform preparing some graphics for the image

      //             ui->wvPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

                     ui->wvPlot->axisRect()->setupFullAxesBox(true);
                     ui->wvPlot->xAxis->setLabel("Time (ms)");
                     ui->wvPlot->yAxis->setLabel("Frequency (Hz)");
                     if(rescale==true){
                     ui->wvPlot->yAxis->setRange(0,*std::max_element(freq.begin(),freq.end()));
                     ui->wvPlot->xAxis->setRange(0,*std::max_element(time.begin(),time.end()));
                     }



                     QCPColorMap *colormap = new QCPColorMap(ui->wvPlot->xAxis,ui->wvPlot->yAxis);


                     colormap->data()->clear();
                     colormap->data()->setSize(tlength,N);
                     colormap->data()->setRange(QCPRange(0,*std::max_element(time.begin(),time.end())),QCPRange(0,*std::max_element(freq.begin(),freq.end())));
    //                 for(int t=0;t<tlength;t++){
                     for(int t=0;t<tlength;t++){
                         for(int i=0;i<N;i++){
        //                     colormap->data()->setCell(t,i,std::real(WV[t*N+i])); // This should be the real value - see papers.
                             colormap->data()->setCell(t,i,std::abs(WV[t*N+i])); //plotting the module of the amplitude instead of the amplitude of the real part looks better on the graph and normalizing the graph from 0 to 1.
                         }
                     }



                     WV.clear();



                     QCPColorScale *colorScale = new QCPColorScale(ui->wvPlot);
                     ui->wvPlot->plotLayout()->addElement(1,1,colorScale);
                     colorScale->setType(QCPAxis::atRight);


                     colormap->setColorScale(colorScale);
                     colormap->setGradient(QCPColorGradient::gpJet);
                     colormap->rescaleDataRange();
                     ui->wvPlot->replot();



}




trace_ex::~trace_ex()
{
    delete ui;
}







void trace_ex::on_checkBox_toggled(bool checked)
{    

    ui->tracePlot->clearItems();
    ui->tracePlot->clearGraphs();
    ui->tracePlot->clearPlottables();
    ui->specPlot->clearItems();
    ui->specPlot->clearGraphs();
    ui->specPlot->clearPlottables();
    ui->wvPlot->clearItems();
    ui->wvPlot->clearGraphs();
    ui->wvPlot->clearPlottables();
    ui->tableWidgettr->clear();

    rescale=false;
    smooth=checked;
    emit wvd_signal(trpos,checked,rescale);

}

void trace_ex::on_horizontalSlider_valueChanged(int value)
{    

    ui->tracePlot->clearItems();
    ui->tracePlot->clearGraphs();
    ui->tracePlot->clearPlottables();
    ui->specPlot->clearItems();
    ui->specPlot->clearGraphs();
    ui->specPlot->clearPlottables();
    ui->wvPlot->clearItems();
    ui->wvPlot->clearGraphs();
    ui->wvPlot->clearPlottables();
    ui->tableWidgettr->clear();

    rescale=true;
    trpos=value;
    emit wvd_signal(value,smooth,rescale);
}

void trace_ex::on_tableWidgettr_cellClicked(int row, int column)
{
    if(column!=2&&column!=3){
        ui->tableWidgettr->item(row,column)->setFlags(ui->tableWidgettr->item(row, column)->flags() & ~Qt::ItemIsEditable);  //This will prevent that we edit columns 0 and 1
    }
}

void trace_ex::on_toolButton_clicked()
{
    qDebug() << "geomChange=" << geomChange;
    if(geomChange==true){
        qDebug() << "remaking geometry";
    geomwin->close();
    geomwin->destroy(true,true);
    geomwin = new geom_ex;
    geomwin->show();}
    emit reloadTrace(trpos,smooth,rescale);

}

void trace_ex::on_tableWidgettr_cellChanged(int row, int column)
{
//    qDebug() << "row=" << row;
    if(column==3){
     if(ui->tableWidgettr->model()->data(ui->tableWidgettr->model()->index(row,column)).toInt()!=4&&ui->tableWidgettr->model()->data(ui->tableWidgettr->model()->index(row,column)).toInt()!=2){
        QMessageBox msgbox;
        msgbox.warning(0,"Warning","Byte can only be 2 or 4");
        ui->tableWidgettr->setItem(row,column,new QTableWidgetItem(tr_bytes_length[row]));
     }
    }

    if(column==2){
     bi[row]=ui->tableWidgettr->model()->data(ui->tableWidgettr->model()->index(row,column)).toInt();
    }
    else if(column==3){
     bl[row]=ui->tableWidgettr->model()->data(ui->tableWidgettr->model()->index(row,column)).toInt();
    }

    if(row==21||row==22||row==23||row==24){geomChange=true;}
    else{geomChange=false;}


}


