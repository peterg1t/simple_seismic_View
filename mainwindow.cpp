#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "trace_ex.h"
#include "trace_group_ex.h"
#include "geom_ex.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <string>
#include <QMessageBox>
#include <QDebug>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMenuBar>
#include <QStatusBar>
#include <QCloseEvent>
#include <QtQuickWidgets/QQuickWidget>
#include <QtLocation>
#include <QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QGuiApplication>
#include <QQmlApplicationEngine>



QString modelname;
char xHD[3200]; //array of char to store EBCDIC header
QString HD[40];   //
QString HD2;
QString HD3;
bool ok;
QByteArray temp;
QByteArray lbyte[100];

int intsample;
int fileformat;
quint32 tlength;
int code;
quint32 filelen;




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //parent->setWindowTitle("WaveMod");
    ui->setupUi(this);
    ui->listWidget->setVisible(false);
    ui->tableWidget->setVisible(false);
    ui->radioButton->setVisible(false);
    ui->radioButton_2->setVisible(false);
    ui->radioButton->setDisabled(true);
    ui->radioButton_2->setDisabled(true);
//    ui->progressBar->setHidden(true);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}



void MainWindow::on_radioButton_toggled(bool checked)
{
    ui->listWidget->setVisible(true);
    ui->tableWidget->setVisible(false);
}


void MainWindow::on_radioButton_2_toggled(bool checked)
{
    int i;
    QStringList headers;
    headers << "values" << "description" <<  "bytes";
    QStringList descr;
    descr << "Job identification number" << "Line number" << "Reel number" << "# data traces per record" << "#aux traces per record" << "Sample interval (microseconds) for reel" << "Sample interval (microseconds) for field" << "Number samples per data trace for reel" << "Number samples per data trace for field" << "Data sample format code" << "Group fold" << "Trace sorting code" << "Vertical sum code" << "Sweep frequency at start (Hz)" << "Sweep frequency at end (Hz)" << "Sweep length (ms)" << "Sweep type code" << "Trace number of sweep channel" << "Sweep trace taper length at start (ms)" << "Sweep trace taper length at end (ms)" << "Taper type" << "Correlated data traces" << "Binary gain recovered" << "Amplitude recovery method" << "Measurement system (1-m / 2-feet)" << "Impulse signal" << "Vibratory polarity code";
    QStringList bytes_desc;
    bytes_desc << "1-4" << "5-8" << "9-12" << "13-14" << "15-16" << "17-18" << "19-20" << "21-22" << "23-24" << "25-26" << "27-28" << "29-30" << "31-32" << "33-34" << "35-36" << "37-38" << "39-40" << "41-42" << "43-44" << "45-46" << "47-48" << "49-50" << "51-52" << "53-54" << "55-56" << "57-58" << "59-60" ;

    ui->listWidget->setVisible(false);
    ui->tableWidget->setVisible(true);

    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setRowCount(27);
    for (i =  0; i < 27; i++) {
    ui->tableWidget->setItem(i,0,new QTableWidgetItem(QString::number(lbyte[i].toHex().toInt(&ok,16))));
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->setItem(i,1, new QTableWidgetItem(descr[i]));
    ui->tableWidget->setItem(i,2, new QTableWidgetItem(bytes_desc[i]));
    ui->tableWidget->QTableView::setColumnWidth(1,250);

    };

}


void MainWindow::on_actionCreate_geometry_from_model_triggered()
{
    // Insert geometry generation here
    exit(0);


//    //Now after the geometry is loaded we display the map
//    geomwin = new geom_ex;
//    geomwin->show();
}



void MainWindow::on_actionAbout_triggered()
{
    QApplication::quit();
}


//section where after removing the main window all other are removed as well
void MainWindow::closeEvent(QCloseEvent *event){
//   event->ignore();
   QApplication::quit();
}



void MainWindow::on_actionExplore_SEGY_by_groups_triggered()
{

    modelname = QFileDialog::getOpenFileName(this,
    tr("Open Model"), "D:/Estudios/Informatica/WaveModeling", tr("Model Files (*.mod *.txt *.segy *.sgy)"));
     QFile f(modelname);
     QDataStream in(&f);
     filelen=(f.size()-3200-400)/(240+4*tlength);
     QMessageBox msgbox;

     if (!f.open(QFile::ReadOnly|QFile::Text)) {
         msgbox.warning(0,"Warning","Please select a SEGY file");
 //        msgbox.setText("The file could not be opened");
 //        msgbox.exec();
         return;
     }


          QByteArray ebcdic(f.read(3200));
          QByteArray reel(f.read(400));
          HD2 = QString::fromLatin1(ebcdic);

 //closing the opened file
          f.close();




 //Creating form were we are going to further read the file
          statusBar()->showMessage(tr("File successfully loaded"),3000); // messageChanged(tr("File successfully loaded"));
          ui->actionCreate_geometry_from_model->setEnabled(true);


 //         This loop will convert the reel binary header into 4-byte numbers we need to mke groups of 4 bytes and convert them into base 10
          int j =0;
          for (int i = 0 ; i < 11 ;i = i + 4) {
          temp.append(reel[i]);
          temp.append(reel[i+1]);
          temp.append(reel[i+2]);
          temp.append(reel[i+3]);
          lbyte[j]=temp;
          temp=0;
          j=j+1;
          }
 //       After bit 12 we have 2-byte numbers
          for (int i = 12 ; i < 60 ;i = i + 2) {
          temp.append(reel[i]);
          temp.append(reel[i+1]);
          lbyte[j]=temp;
          temp=0;
          j=j+1;
          }


 //      Reading from the binary header sample integers and trace length
          intsample=lbyte[5].toHex().toInt(&ok,16);
          qDebug() << intsample;
          tlength=lbyte[7].toHex().toInt(&ok,16);
          code=lbyte[9].toHex().toInt(&ok,16);
          qDebug() << "Format code" << code;
          qDebug() << "Trace length" << tlength;
          //    launching trace_exp (trace explorer window) as well with trace number one once we have the file and trace length.
          tracewingrp = new trace_group_ex;
          tracewingrp->show();


//          QQuickWidget *view = new QQuickWidget;
//          view->setSource(QUrl::fromLocalFile("geom_map.qml"));
//          view->show();
//          qDebug() << "here";




 //         Reading the EBCDIC header
       for (int i = 0; i < 3199; i = i + 1) {

           //    !NUMEROS

                 if(HD2.at(i)== QString("\u00f1")) {xHD[i]='1';}
                 if(HD2.at(i)== QString("\u00f2")) {xHD[i]='2';}
                 if(HD2.at(i)== QString("\u00f3")) {xHD[i]='3';}
                 if(HD2.at(i)== QString("\u00f4")) {xHD[i]='4';}
                 if(HD2.at(i)== QString("\u00f5")) {xHD[i]='5';}
                 if(HD2.at(i)== QString("\u00f6")) {xHD[i]='6';}
                 if(HD2.at(i)== QString("\u00f7")) {xHD[i]='7';}
                 if(HD2.at(i)== QString("\u00f8")) {xHD[i]='8';}
                 if(HD2.at(i)== QString("\u00f9")) {xHD[i]='9';}
                 if(HD2.at(i)== QString("\u00f0")) {xHD[i]='0';}




             //     !ALGUNOS SIMBOLOS
                 if(HD2.at(i)== QString("a")) {xHD[i]='/';}
                 if(HD2.at(i)== QString("\u00e0")) {xHD[i]='\\';}
                 if(HD2.at(i)== QString("`")) {xHD[i]='-';}
                 if(HD2.at(i)== QString("\u007a")) {xHD[i]=':';}
                 if(HD2.at(i)== QString("K")) {xHD[i]='.';}
                 if(HD2.at(i)== QString("m")) {xHD[i]='_';}
                 if(HD2.at(i)== QString("k")) {xHD[i]=',';}
                 if(HD2.at(i)== QString("\u004d")) {xHD[i]='(';}
                 if(HD2.at(i)== QString("\u005d")) {xHD[i]=')';}
                 if(HD2.at(i)== QString("O")) {xHD[i]='!';}
                 if(HD2.at(i)== QString("|")) {xHD[i]='@';}
                 if(HD2.at(i)== QString("{")) {xHD[i]='#';}
                 if(HD2.at(i)== QString("[")) {xHD[i]='$';}
                 if(HD2.at(i)== QString("l")) {xHD[i]='%';}
                 if(HD2.at(i)== QString("_")) {xHD[i]='^';}
                 if(HD2.at(i)== QString("P")) {xHD[i]='&';}
  //                if(HD2.at(i)== QString("\")) {xHD[i]='*';}
                 if(HD2.at(i)== QString("M")) {xHD[i]='(';}
                 if(HD2.at(i)== QString("]")) {xHD[i]=')';}



               //  !MAYUSCULAS
                 if(HD2.at(i)== QString("\u00c1")) {xHD[i]='A';}
                 if(HD2.at(i)== QString("\u00c2")) {xHD[i]='B';}
                 if(HD2.at(i)== QString("\u00c3")) {xHD[i]='C';}
                 if(HD2.at(i)== QString("\u00c4")) {xHD[i]='D';}
                 if(HD2.at(i)== QString("\u00c5")) {xHD[i]='E';}
                 if(HD2.at(i)== QString("\u00c6")) {xHD[i]='F';}
                 if(HD2.at(i)== QString("\u00c7")) {xHD[i]='G';}
                 if(HD2.at(i)== QString("\u00c8")) {xHD[i]='H';}
                 if(HD2.at(i)== QString("\u00c9")) {xHD[i]='I';}
                 if(HD2.at(i)== QString("\u00d1")) {xHD[i]='J';}
                 if(HD2.at(i)== QString("\u00d2")) {xHD[i]='K';}
                 if(HD2.at(i)== QString("\u00d3")) {xHD[i]='L';}
                 if(HD2.at(i)== QString("\u00d4")) {xHD[i]='M';}
                 if(HD2.at(i)== QString("\u00d5")) {xHD[i]='N';}
             //    !if(xHD[i]==L' ') xHD[i]='Ñ';
                 if(HD2.at(i)== QString("\u00d6")) {xHD[i]='O';}
                 if(HD2.at(i)== QString("\u00d7")) {xHD[i]='P';}
                 if(HD2.at(i)== QString("\u00d8")) {xHD[i]='Q';}
                 if(HD2.at(i)== QString("\u00d9")) {xHD[i]='R';}
                 if(HD2.at(i)== QString("\u00e2")) {xHD[i]='S';}
                 if(HD2.at(i)== QString("\u00e3")) {xHD[i]='T';}
                 if(HD2.at(i)== QString("\u00e4")) {xHD[i]='U';}
                 if(HD2.at(i)== QString("\u00e5")) {xHD[i]='V';}
                 if(HD2.at(i)== QString("\u00e6")) {xHD[i]='W';}
                 if(HD2.at(i)== QString("\u00e7")) {xHD[i]='X';}
                 if(HD2.at(i)== QString("\u00e8")) {xHD[i]='Y';}
                 if(HD2.at(i)== QString("\u00e9")) {xHD[i]='Z';}



            //      !MINUSCULAS

                 if(HD2.at(i)== QString("\u0081")) {xHD[i]='a';}
                 if(HD2.at(i)== QString("\u0082")) {xHD[i]='b';}
                 if(HD2.at(i)== QString("\u0083")) {xHD[i]='c';}
                 if(HD2.at(i)== QString("\u0084")) {xHD[i]='d';}
                 if(HD2.at(i)== QString("\u0085")) {xHD[i]='e';}
                 if(HD2.at(i)== QString("\u0086")) {xHD[i]='f';}
                 if(HD2.at(i)== QString("\u0087")) {xHD[i]='g';}
                 if(HD2.at(i)== QString("\u0088")) {xHD[i]='h';}
                 if(HD2.at(i)== QString("\u0089")) {xHD[i]='i';}
                 if(HD2.at(i)== QString("\u0091")) {xHD[i]='j';}
                 if(HD2.at(i)== QString("\u0092")) {xHD[i]='k';}
                 if(HD2.at(i)== QString("\u0093")) {xHD[i]='l';}
                 if(HD2.at(i)== QString("\u0094")) {xHD[i]='m';}
                 if(HD2.at(i)== QString("\u0095")) {xHD[i]='n';}
                 //           !if(xHD[i]==L'=') xHD[i]='ñ';
                 if(HD2.at(i)== QString("\u0096")) {xHD[i]='o';}
                 if(HD2.at(i)== QString("\u0097")) {xHD[i]='p';}
                 if(HD2.at(i)== QString("\u0098")) {xHD[i]='q';}
                 if(HD2.at(i)== QString("\u0099")) {xHD[i]='r';}
                 if(HD2.at(i)== QString("\u00a2")) {xHD[i]='s';}
                 if(HD2.at(i)== QString("\u00a3")) {xHD[i]='t';}
                 if(HD2.at(i)== QString("\u00a4")) {xHD[i]='u';}
                 if(HD2.at(i)== QString("\u00a5")) {xHD[i]='v';}
                 if(HD2.at(i)== QString("\u00a6")) {xHD[i]='w';}
                 if(HD2.at(i)== QString("\u00a7")) {xHD[i]='x';}
                 if(HD2.at(i)== QString("\u00a8")) {xHD[i]='y';}
                 if(HD2.at(i)== QString("\u00a9")) {xHD[i]='z';}



 //                 !CARACTER ESPECIAL
                 if(HD2.at(i)== QString("@")) {xHD[i]=' ';}



 //                qDebug() << "EBCDIC bytes"  << xHD[i] << HD2.at(i)  << i ;

        };




 //    Cleaning the listview and the header array to plot the new file
       ui->listWidget->clear();
       for (int i = 0; i < 40; i = i + 1){HD[i]="";};



 //    Writing to the list view the header
       int k = 0;
       for (int i = 0; i < 40; i = i + 1){
       for (int j = 0; j < 80; j = j + 1) {
           HD[i] += QString(xHD[k]);
           k = k + 1;
       };

       ui->listWidget->addItem(HD[i]);
       };



         f.close();

         ui->listWidget->setVisible(true);
         ui->radioButton->setVisible(true);
         ui->radioButton_2->setVisible(true);
         ui->radioButton->setChecked(true);
         ui->radioButton_2->setChecked(false);
         ui->radioButton->setEnabled(true);
         ui->radioButton_2->setEnabled(true);


}

void MainWindow::on_actionExplore_seq_triggered()
{


    modelname = QFileDialog::getOpenFileName(this,
    tr("Open Model"), "D:/Estudios/Informatica/WaveModeling", tr("Model Files (*.mod *.txt *.segy *.sgy)"));
     QFile f(modelname);
     QDataStream in(&f);
     QMessageBox msgbox;

     if (!f.open(QFile::ReadOnly|QFile::Text)) {
         msgbox.warning(0,"Warning","Please select a SEGY file");
 //        msgbox.setText("The file could not be opened");
 //        msgbox.exec();
         return;
     }


          QByteArray ebcdic(f.read(3200));
          QByteArray reel(f.read(400));
          HD2 = QString::fromLatin1(ebcdic);

 //closing the opened file
          f.close();




 //Creating form were we are going to further read the file
          statusBar()->showMessage(tr("File successfully loaded"),3000); // messageChanged(tr("File successfully loaded"));
          ui->actionCreate_geometry_from_model->setEnabled(true);


 //         This loop will convert the reel binary header into 4-byte numbers we need to mke groups of 4 bytes and convert them into base 10
          int j =0;
          for (int i = 0 ; i < 11 ;i = i + 4) {
          temp.append(reel[i]);
          temp.append(reel[i+1]);
          temp.append(reel[i+2]);
          temp.append(reel[i+3]);
          lbyte[j]=temp;
          temp=0;
          j=j+1;
          }
 //       After bit 12 we have 2-byte numbers
          for (int i = 12 ; i < 60 ;i = i + 2) {
          temp.append(reel[i]);
          temp.append(reel[i+1]);
          lbyte[j]=temp;
          temp=0;
          j=j+1;
          }

 //      Reading from the binary header sample integers and trace length
          intsample=lbyte[5].toHex().toInt(&ok,16);
          qDebug() << intsample;
          tlength=lbyte[7].toHex().toInt(&ok,16);
          code=lbyte[9].toHex().toInt(&ok,16);
          qDebug() << "Format code" << code;
          qDebug() << "Trace length" << tlength;
          //    launching trace_exp (trace explorer window) as well with trace number one once we have the file and trace length.
          tracewin = new trace_ex;
          tracewin->traceread(1);
          geomwin = new geom_ex;
          geomwin->show();
          tracewin->show();


//          QQuickWidget *view = new QQuickWidget;   //Enable this to displaymap example with QTQuick
//          view->setSource(QUrl::fromLocalFile("geom_map.qml"));






 //         Reading the EBCDIC header
       for (int i = 0; i < 3199; i = i + 1) {

           //    !NUMEROS

                 if(HD2.at(i)== QString("\u00f1")) {xHD[i]='1';}
                 if(HD2.at(i)== QString("\u00f2")) {xHD[i]='2';}
                 if(HD2.at(i)== QString("\u00f3")) {xHD[i]='3';}
                 if(HD2.at(i)== QString("\u00f4")) {xHD[i]='4';}
                 if(HD2.at(i)== QString("\u00f5")) {xHD[i]='5';}
                 if(HD2.at(i)== QString("\u00f6")) {xHD[i]='6';}
                 if(HD2.at(i)== QString("\u00f7")) {xHD[i]='7';}
                 if(HD2.at(i)== QString("\u00f8")) {xHD[i]='8';}
                 if(HD2.at(i)== QString("\u00f9")) {xHD[i]='9';}
                 if(HD2.at(i)== QString("\u00f0")) {xHD[i]='0';}




             //     !ALGUNOS SIMBOLOS
                 if(HD2.at(i)== QString("a")) {xHD[i]='/';}
                 if(HD2.at(i)== QString("\u00e0")) {xHD[i]='\\';}
                 if(HD2.at(i)== QString("`")) {xHD[i]='-';}
                 if(HD2.at(i)== QString("\u007a")) {xHD[i]=':';}
                 if(HD2.at(i)== QString("K")) {xHD[i]='.';}
                 if(HD2.at(i)== QString("m")) {xHD[i]='_';}
                 if(HD2.at(i)== QString("k")) {xHD[i]=',';}
                 if(HD2.at(i)== QString("\u004d")) {xHD[i]='(';}
                 if(HD2.at(i)== QString("\u005d")) {xHD[i]=')';}
                 if(HD2.at(i)== QString("O")) {xHD[i]='!';}
                 if(HD2.at(i)== QString("|")) {xHD[i]='@';}
                 if(HD2.at(i)== QString("{")) {xHD[i]='#';}
                 if(HD2.at(i)== QString("[")) {xHD[i]='$';}
                 if(HD2.at(i)== QString("l")) {xHD[i]='%';}
                 if(HD2.at(i)== QString("_")) {xHD[i]='^';}
                 if(HD2.at(i)== QString("P")) {xHD[i]='&';}
  //                if(HD2.at(i)== QString("\")) {xHD[i]='*';}
                 if(HD2.at(i)== QString("M")) {xHD[i]='(';}
                 if(HD2.at(i)== QString("]")) {xHD[i]=')';}



               //  !MAYUSCULAS
                 if(HD2.at(i)== QString("\u00c1")) {xHD[i]='A';}
                 if(HD2.at(i)== QString("\u00c2")) {xHD[i]='B';}
                 if(HD2.at(i)== QString("\u00c3")) {xHD[i]='C';}
                 if(HD2.at(i)== QString("\u00c4")) {xHD[i]='D';}
                 if(HD2.at(i)== QString("\u00c5")) {xHD[i]='E';}
                 if(HD2.at(i)== QString("\u00c6")) {xHD[i]='F';}
                 if(HD2.at(i)== QString("\u00c7")) {xHD[i]='G';}
                 if(HD2.at(i)== QString("\u00c8")) {xHD[i]='H';}
                 if(HD2.at(i)== QString("\u00c9")) {xHD[i]='I';}
                 if(HD2.at(i)== QString("\u00d1")) {xHD[i]='J';}
                 if(HD2.at(i)== QString("\u00d2")) {xHD[i]='K';}
                 if(HD2.at(i)== QString("\u00d3")) {xHD[i]='L';}
                 if(HD2.at(i)== QString("\u00d4")) {xHD[i]='M';}
                 if(HD2.at(i)== QString("\u00d5")) {xHD[i]='N';}
             //    !if(xHD[i]==L' ') xHD[i]='Ñ';
                 if(HD2.at(i)== QString("\u00d6")) {xHD[i]='O';}
                 if(HD2.at(i)== QString("\u00d7")) {xHD[i]='P';}
                 if(HD2.at(i)== QString("\u00d8")) {xHD[i]='Q';}
                 if(HD2.at(i)== QString("\u00d9")) {xHD[i]='R';}
                 if(HD2.at(i)== QString("\u00e2")) {xHD[i]='S';}
                 if(HD2.at(i)== QString("\u00e3")) {xHD[i]='T';}
                 if(HD2.at(i)== QString("\u00e4")) {xHD[i]='U';}
                 if(HD2.at(i)== QString("\u00e5")) {xHD[i]='V';}
                 if(HD2.at(i)== QString("\u00e6")) {xHD[i]='W';}
                 if(HD2.at(i)== QString("\u00e7")) {xHD[i]='X';}
                 if(HD2.at(i)== QString("\u00e8")) {xHD[i]='Y';}
                 if(HD2.at(i)== QString("\u00e9")) {xHD[i]='Z';}



            //      !MINUSCULAS

                 if(HD2.at(i)== QString("\u0081")) {xHD[i]='a';}
                 if(HD2.at(i)== QString("\u0082")) {xHD[i]='b';}
                 if(HD2.at(i)== QString("\u0083")) {xHD[i]='c';}
                 if(HD2.at(i)== QString("\u0084")) {xHD[i]='d';}
                 if(HD2.at(i)== QString("\u0085")) {xHD[i]='e';}
                 if(HD2.at(i)== QString("\u0086")) {xHD[i]='f';}
                 if(HD2.at(i)== QString("\u0087")) {xHD[i]='g';}
                 if(HD2.at(i)== QString("\u0088")) {xHD[i]='h';}
                 if(HD2.at(i)== QString("\u0089")) {xHD[i]='i';}
                 if(HD2.at(i)== QString("\u0091")) {xHD[i]='j';}
                 if(HD2.at(i)== QString("\u0092")) {xHD[i]='k';}
                 if(HD2.at(i)== QString("\u0093")) {xHD[i]='l';}
                 if(HD2.at(i)== QString("\u0094")) {xHD[i]='m';}
                 if(HD2.at(i)== QString("\u0095")) {xHD[i]='n';}
                 //           !if(xHD[i]==L'=') xHD[i]='ñ';
                 if(HD2.at(i)== QString("\u0096")) {xHD[i]='o';}
                 if(HD2.at(i)== QString("\u0097")) {xHD[i]='p';}
                 if(HD2.at(i)== QString("\u0098")) {xHD[i]='q';}
                 if(HD2.at(i)== QString("\u0099")) {xHD[i]='r';}
                 if(HD2.at(i)== QString("\u00a2")) {xHD[i]='s';}
                 if(HD2.at(i)== QString("\u00a3")) {xHD[i]='t';}
                 if(HD2.at(i)== QString("\u00a4")) {xHD[i]='u';}
                 if(HD2.at(i)== QString("\u00a5")) {xHD[i]='v';}
                 if(HD2.at(i)== QString("\u00a6")) {xHD[i]='w';}
                 if(HD2.at(i)== QString("\u00a7")) {xHD[i]='x';}
                 if(HD2.at(i)== QString("\u00a8")) {xHD[i]='y';}
                 if(HD2.at(i)== QString("\u00a9")) {xHD[i]='z';}



 //                 !CARACTER ESPECIAL
                 if(HD2.at(i)== QString("@")) {xHD[i]=' ';}



 //                qDebug() << "EBCDIC bytes"  << xHD[i] << HD2.at(i)  << i ;

        };




 //    Cleaning the listview and the header array to plot the new file
       ui->listWidget->clear();
       for (int i = 0; i < 40; i = i + 1){HD[i]="";};



 //    Writing to the list view the header
       int k = 0;
       for (int i = 0; i < 40; i = i + 1){
       for (int j = 0; j < 80; j = j + 1) {
           HD[i] += QString(xHD[k]);
           k = k + 1;
       };

       ui->listWidget->addItem(HD[i]);
       };



         f.close();

         ui->listWidget->setVisible(true);
         ui->radioButton->setVisible(true);
         ui->radioButton_2->setVisible(true);
         ui->radioButton->setChecked(true);
         ui->radioButton_2->setChecked(false);
         ui->radioButton->setEnabled(true);
         ui->radioButton_2->setEnabled(true);
}




