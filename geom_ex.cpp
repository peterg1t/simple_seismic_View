#include "geom_ex.h"
#include "ui_geom_ex.h"
#include <QtCharts>
#include <QScatterSeries>
#include <QVector>
#include <QByteArray>
#include <qcustomplot.h>
#include <mainwindow.h>
#include <QStatusBar>



using namespace QtCharts;  //used when using charts, although simpler I will use qcustomplot to take advantage of zoomimg/panning capabilities



extern QString modelname;
extern quint32 tlength;
extern quint32 fillen;
QByteArray hd_temp2;
QByteArray tr_lbyte2[100];
QByteArray thead;
//WORKING HERE defining 3 columns array for the shot and receiver tables the QScatterSeries will be built after the tables have been created???!!
//creation of the tables will be based on the standard shot and receiver data in segy locations
// shotpoint 17-20  <- this is the key value for the table
// shotx 73-76
// shoty 77-80
// recx 81-84
// recy 85-88



geom_ex::geom_ex(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::geom_ex)
{
    ui->setupUi(this);



// After opening the map window we are going to read the entire SEGY and read from the segy standard all the shots locations.

    QFile f(modelname);
    f.open(QFile::ReadOnly);

    QVector2D sp_coord((fillen-3200-400)/(240+4*tlength)-1,(fillen-3200-400)/(240+4*tlength)-1);
//    QVector<double> spn((fillen-3200-400)/(240+4*tlength)-1); //Initializing vectors for shot number, shotx and shoty. fillen is the number of traces in the segy calculated in mainwindow.cpp
//    QVector<double> spx((fillen-3200-400)/(240+4*tlength)-1), spy((fillen-3200-400)/(240+4*tlength)-1);
//    QVector<double> recx((fillen-3200-400)/(240+4*tlength)-1), recy((fillen-3200-400)/(240+4*tlength)-1); //Initializing vectors for shot number, shotx and shoty. fillen is the number of traces in the segy calculated in mainwindow.cpp

    QVector<double> spn(0); //Initializing vectors for shot number, shotx and shoty. fillen is the number of traces in the segy calculated in mainwindow.cpp
    QVector<double> spx(0), spy(0);
    QVector<double> recx(0), recy(0); //Initializing vectors for shot number, shotx and shoty. fillen is the number of traces in the segy calculated in mainwindow.cpp


    quint32 kval=1;
    do{
        f.seek(3200+400+(240+4*tlength)*(kval-1));
        thead=f.read(240);

                // This section calculates the bytes to display the header attributes (4-bytes words)
                int j =0;
                for (int i = 0 ; i < 27 ;i = i + 4) {
                hd_temp2.append(thead[i]);
                hd_temp2.append(thead[i+1]);
                hd_temp2.append(thead[i+2]);
                hd_temp2.append(thead[i+3]);
//                qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp2) << hd_temp2.toHex().toInt(nullptr,16) << i ;
                tr_lbyte2[j]=hd_temp2;    
                hd_temp2=0;
                j=j+1;
                }
//                qDebug() << tr_lbyte2[4].toHex().toInt(nullptr,16) << k << (fillen-3200-400)/(240+4*tlength)-1 ;

                // This section calculates the bytes to display the header attributes (2-bytes words)

                for (int i = 28 ; i < 35 ;i = i + 2) {
                hd_temp2.append(thead[i]);
                hd_temp2.append(thead[i+1]);
//                 hd_temp2.append(thead[i+2]);
//                 hd_temp2.append(thead[i+3]);
//                 qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp2) << hd_temp2.toHex().toInt(nullptr,16) << i ;
                tr_lbyte2[j]=hd_temp2;
                hd_temp2=0;
                j=j+1;
                }


                // from 36 to 69 they are 4-byte words again

                for (int i = 36 ; i < 67 ;i = i + 4) {
                hd_temp2.append(thead[i]);
                hd_temp2.append(thead[i+1]);
                hd_temp2.append(thead[i+2]);
                hd_temp2.append(thead[i+3]);
       //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(temp2) << temp2.toHex().toInt(&ok,16) << i ;
                tr_lbyte2[j]=hd_temp2;
                hd_temp2=0;
                j=j+1;
                }



                // from 69 to 71 they are 2-byte words again (scalars)
                for (int i = 68 ; i < 71 ;i = i + 2) {
                hd_temp2.append(thead[i]);
                hd_temp2.append(thead[i+1]);
//                 hd_temp2.append(thead[i+2]);
//                 hd_temp2.append(thead[i+3]);
//                 QDataStream  <--
//                 qDebug() << "Bytes after EBCDIC"  << static_cast<int16_t>(hd_temp2.toHex().toInt(nullptr,16)) << i ;
                tr_lbyte2[j]=hd_temp2;
                hd_temp2=0;
                j=j+1;
                }



                // from 72 to 89 they are 4-byte words again

                for (int i = 72 ; i < 87 ;i = i + 4) {
                hd_temp2.append(thead[i]);
                hd_temp2.append(thead[i+1]);
                hd_temp2.append(thead[i+2]);
                hd_temp2.append(thead[i+3]);
//                qDebug() << "Bytes after EBCDIC source and group coordinates"  << QString::fromLocal8Bit(hd_temp2) << hd_temp2.toHex().toInt(nullptr,16) << i << j;
//                qDebug() << (fillen-3200-400)/(240+4*tlength)-1;
                tr_lbyte2[j]=hd_temp2;
                hd_temp2=0;
                j=j+1;
                }


                if(spn.indexOf(tr_lbyte2[4].toHex().toInt(nullptr,16))==-1){
                    spn.append(tr_lbyte2[4].toHex().toInt(nullptr,16));
                    spx.append(tr_lbyte2[21].toHex().toInt(nullptr,16));
                    spy.append(tr_lbyte2[22].toHex().toInt(nullptr,16));
                }

                if(recx.indexOf(tr_lbyte2[23].toHex().toInt(nullptr,16))==-1 && recy.indexOf(tr_lbyte2[24].toHex().toInt(nullptr,16))==-1){
                    recx.append(tr_lbyte2[23].toHex().toInt(nullptr,16));
                    recy.append(tr_lbyte2[24].toHex().toInt(nullptr,16));
                }



                // from 89 to 144 they are 2-byte words again

                for (int i = 88 ; i < 143 ;i = i + 2) {
                hd_temp2.append(thead[i]);
                hd_temp2.append(thead[i+1]);
//                 hd_temp2.append(thead[i+2]);
//                 hd_temp2.append(thead[i+3]);
       //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(temp2) << temp2.toHex().toInt(&ok,16) << i ;
                tr_lbyte2[j]=hd_temp2;
                hd_temp2=0;
                j=j+1;
                }



                // from 148 to 179 they are 2-byte words again

                for (int i = 148 ; i < 179 ;i = i + 2) {
                hd_temp2.append(thead[i]);
                hd_temp2.append(thead[i+1]);
//                 hd_temp2.append(thead[i+2]);
//                 hd_temp2.append(thead[i+3]);
//                qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp2) << hd_temp2.toHex().toInt(nullptr,16) << i ;
                tr_lbyte2[j]=hd_temp2;
                hd_temp2=0;
                j=j+1;
                }

//                qDebug() << "k=" <<k ;
                kval=kval+1;


//    }while(f.atEnd());
    }while(kval<=(fillen-3200-400)/(240+4*tlength)-1 );
//      }while(k<=280);



//    for(int i=0;i<spn.size();i++){
//         qDebug() << i << spn.at(i) << spx.at(i) << spy.at(i) ;
//    }



//  Adding graph with all the shots and the receivers
    QPen pen;
    pen.setWidth(1);
    pen.setColor(QColor(255,0,0,255));
    ui->mapplot->yAxis->setLabel("Northing (m)"); //adding labels
    ui->mapplot->xAxis->setLabel("Easting (m)");
    ui->mapplot->addGraph();
    ui->mapplot->graph(0)->setScatterStyle(QCPScatterStyle::ssDisc);
    ui->mapplot->graph(0)->setLineStyle(QCPGraph::lsNone);
    ui->mapplot->graph(0)->setPen(pen);
    ui->mapplot->graph(0)->addData(spx,spy);  //check QCPMapData
    ui->mapplot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->mapplot->axisRect(0)->setRangeDrag(Qt::Horizontal|Qt::Vertical);
    ui->mapplot->axisRect(0)->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    ui->mapplot->rescaleAxes();

    QPen recpen;
    recpen.setWidth(1);
    recpen.setColor(QColor(0,0,255,255));
    ui->mapplot->addGraph();
    ui->mapplot->graph(1)->setScatterStyle(QCPScatterStyle::ssDiamond);
    ui->mapplot->graph(1)->setLineStyle(QCPGraph::lsNone);
    ui->mapplot->graph(1)->setPen(recpen);
    ui->mapplot->graph(1)->addData(recx,recy);  //check QCPMapData
//    ui->mapplot->axisRect(1)->setRangeDrag(Qt::Horizontal|Qt::Vertical);
//    ui->mapplot->axisRect(1)->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    ui->mapplot->rescaleAxes();




      connect(ui->mapplot,&QCustomPlot::mousePress, this, &geom_ex::slotMousePress);
      connect(ui->mapplot,&QCustomPlot::mouseMove, this, &geom_ex::slotMouseMove);


}







//void geom_ex::mousePressEvent(QMouseEvent *ev){
//    ui->mapplot->mousePress(ev);
//  qDebug() << ui->mapplot->mapFromGlobal(QCursor::pos()) << ui->mapplot->xAxis->pixelToCoord(ev->pos().x()) ;
//}


void geom_ex::slotMousePress(QMouseEvent *ev){

}

void geom_ex::slotMouseMove(QMouseEvent *ev){
    long xc = ui->mapplot->xAxis->pixelToCoord(ev->pos().x());
    long yc = ui->mapplot->yAxis->pixelToCoord(ev->pos().y());

    ui->xcoord->setText("X="+QString::number(xc,10));
    ui->ycoord->setText("Y="+QString::number(yc,10));
//    qDebug() << ui->mapplot->xAxis->pixelToCoord(ev->pos().x()) << ui->mapplot->yAxis->pixelToCoord(ev->pos().y());
}


geom_ex::~geom_ex()
{
    delete ui;
}
