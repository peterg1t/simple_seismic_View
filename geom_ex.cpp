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
extern int unit;
QByteArray hd_temp2;
QByteArray tr_lbyte2[100];
QByteArray thead;
//defining 3 columns array for the shot and receiver tables the QScatterSeries will be built after the tables have been created???!!
//creation of the tables will be based on the standard shot and receiver data in segy locations
// shotpoint 17-20  <- this is the key value for the table
// shotx 73-76
// shoty 77-80
// recx 81-84
// recy 85-88
int numpoints=0;
double xi;
double yi;
double xe;
double ye;
double xm;
double ym;
extern int bi[];
extern int bl[];



geom_ex::geom_ex(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::geom_ex)
{
    ui->setupUi(this);

    ui->measure->setCheckable(true);
    mapread(bi,bl);
    connect(ui->mapplot,&QCustomPlot::mouseMove, this, &geom_ex::slotMouseMove);

}


void geom_ex::mapread(int bi[], int bl[]){
    // After opening the map window we are going to read the entire SEGY and read from the segy standard all the shots locations.

        QFile f(modelname);
        f.open(QFile::ReadOnly);

        qDebug() << "modelname=" << modelname;


        QVector<quint32> spn(0),recn(0); //Initializing vectors for shot number, shotx and shoty. fillen is the number of traces in the segy calculated in mainwindow.cpp
        QVector<double> spx(0), spy(0);
        QVector<double> recx(0), recy(0); //Initializing vectors for shot number, shotx and shoty. fillen is the number of traces in the segy calculated in mainwindow.cpp
        QVector<double> recelev(0); // Initializing vetor for the elevation profile


        QVector<double> spxh(0),spyh(0);
        QVector<double> recxh(0),recyh(0);

        quint32 kval=1;


        do{
            f.seek(3200+400+(240+4*tlength)*(kval-1));
            thead=f.read(240);

            for(int k=0;k<69;k++){
              for(int i=bi[k]-1;i<bi[k]+bl[k]-1;i++){
                  hd_temp2.append(thead[i]);
//                                qDebug()<< "Here2" <<  i << k << bi[k]-1 << bi[k]+bl[k]-1 << hd_temp2 ;
              }
              tr_lbyte2[k]=hd_temp2;
//                         qDebug()<< k << hd_temp <<  QString::number(static_cast<int16_t>(tr_lbyte[k].toHex().toInt(nullptr,32))) ;
              hd_temp2=0;
            }


            double scalar;
            scalar = static_cast<qint16>(tr_lbyte2[20].toHex().toInt(nullptr,16));
            if(scalar<0){scalar=1./qAbs(scalar);}
            else if (scalar==0) {scalar=1;}

            double scalar_elev;
            scalar_elev = static_cast<qint16>(tr_lbyte2[19].toHex().toInt(nullptr,16));
            if(scalar_elev<0){scalar_elev=1./qAbs(scalar_elev);}
            else if (scalar_elev==0) {scalar_elev=1;}






    //                // This section calculates the bytes to display the header attributes (4-bytes words)
    //                int j =0;
    //                for (int i = 0 ; i < 27 ;i = i + 4) {
    //                hd_temp2.append(thead[i]);
    //                hd_temp2.append(thead[i+1]);
    //                hd_temp2.append(thead[i+2]);
    //                hd_temp2.append(thead[i+3]);
    ////                qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp2) << hd_temp2.toHex().toInt(nullptr,16) << i ;
    //                tr_lbyte2[j]=hd_temp2;
    //                hd_temp2=0;
    //                j=j+1;
    //                }
    ////                qDebug() << tr_lbyte2[4].toHex().toInt(nullptr,16) << k << (fillen-3200-400)/(240+4*tlength)-1 ;

    //                // This section calculates the bytes to display the header attributes (2-bytes words)

    //                for (int i = 28 ; i < 35 ;i = i + 2) {
    //                hd_temp2.append(thead[i]);
    //                hd_temp2.append(thead[i+1]);
    ////                 hd_temp2.append(thead[i+2]);
    ////                 hd_temp2.append(thead[i+3]);
    ////                 qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp2) << hd_temp2.toHex().toInt(nullptr,16) << i ;
    //                tr_lbyte2[j]=hd_temp2;
    //                hd_temp2=0;
    //                j=j+1;
    //                }


    //                // from 36 to 69 they are 4-byte words again

    //                for (int i = 36 ; i < 67 ;i = i + 4) {
    //                hd_temp2.append(thead[i]);
    //                hd_temp2.append(thead[i+1]);
    //                hd_temp2.append(thead[i+2]);
    //                hd_temp2.append(thead[i+3]);
    //       //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(temp2) << temp2.toHex().toInt(&ok,16) << i ;
    //                tr_lbyte2[j]=hd_temp2;
    //                hd_temp2=0;
    //                j=j+1;
    //                }



    //                // from 69 to 71 they are 2-byte words again (scalars)
    //                for (int i = 68 ; i < 71 ;i = i + 2) {
    //                hd_temp2.append(thead[i]);
    //                hd_temp2.append(thead[i+1]);
    ////                 hd_temp2.append(thead[i+2]);
    ////                 hd_temp2.append(thead[i+3]);
    ////                 QDataStream  <--
    ////                 qDebug() << "Bytes after EBCDIC"  << static_cast<int16_t>(hd_temp2.toHex().toInt(nullptr,16)) << i ;
    //                tr_lbyte2[j]=hd_temp2;
    //                hd_temp2=0;
    //                j=j+1;
    //                }



    //                // from 72 to 89 they are 4-byte words again

    //                for (int i = 72 ; i < 87 ;i = i + 4) {
    //                hd_temp2.append(thead[i]);
    //                hd_temp2.append(thead[i+1]);
    //                hd_temp2.append(thead[i+2]);
    //                hd_temp2.append(thead[i+3]);
    ////                qDebug() << "Bytes after EBCDIC source and group coordinates"  << QString::fromLocal8Bit(hd_temp2) << hd_temp2.toHex().toInt(nullptr,16) << i << j;
    ////                qDebug() << (fillen-3200-400)/(240+4*tlength)-1;
    //                tr_lbyte2[j]=hd_temp2;
    //                hd_temp2=0;
    //                j=j+1;
    //                }



                   quint32 temp_spn = tr_lbyte2[21].toHex().toInt(nullptr,16) * pow(10,log10(tr_lbyte2[22].toHex().toInt(nullptr,16))+1)  + tr_lbyte2[22].toHex().toInt(nullptr,16);
                   if(spn.indexOf(temp_spn)==-1){
                        spn.append(temp_spn);
                        spx.append(tr_lbyte2[21].toHex().toInt(nullptr,16)*scalar);
                        spy.append(tr_lbyte2[22].toHex().toInt(nullptr,16)*scalar);
//                        qDebug() << tr_lbyte2[21].toHex().toInt(nullptr,16) << tr_lbyte2[22].toHex().toInt(nullptr,16) << tr_lbyte2[23].toHex().toInt(nullptr,16) << tr_lbyte2[24].toHex().toInt(nullptr,16);
                    }



                    quint32 temp_recn = tr_lbyte2[23].toHex().toInt(nullptr,16) * pow(10,log10(tr_lbyte2[24].toHex().toInt(nullptr,16))+1)  + tr_lbyte2[24].toHex().toInt(nullptr,16);
                    if(recn.indexOf(temp_recn)==-1){
                        recn.append(temp_recn);
                        recx.append(tr_lbyte2[23].toHex().toInt(nullptr,16)*scalar);
                        recy.append(tr_lbyte2[24].toHex().toInt(nullptr,16)*scalar);
                        recelev.append(tr_lbyte2[12].toHex().toInt(nullptr,16)*scalar_elev);
//                        qDebug() << tr_lbyte2[12].toHex().toInt(nullptr,16);
                    }






    //                // from 89 to 144 they are 2-byte words again

    //                for (int i = 88 ; i < 143 ;i = i + 2) {
    //                hd_temp2.append(thead[i]);
    //                hd_temp2.append(thead[i+1]);
    ////                 hd_temp2.append(thead[i+2]);
    ////                 hd_temp2.append(thead[i+3]);
    //       //         qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(temp2) << temp2.toHex().toInt(&ok,16) << i ;
    //                tr_lbyte2[j]=hd_temp2;
    //                hd_temp2=0;
    //                j=j+1;
    //                }



    //                // from 148 to 179 they are 2-byte words again

    //                for (int i = 148 ; i < 179 ;i = i + 2) {
    //                hd_temp2.append(thead[i]);
    //                hd_temp2.append(thead[i+1]);
    ////                 hd_temp2.append(thead[i+2]);
    ////                 hd_temp2.append(thead[i+3]);
    ////                qDebug() << "Bytes after EBCDIC"  << QString::fromLocal8Bit(hd_temp2) << hd_temp2.toHex().toInt(nullptr,16) << i ;
    //                tr_lbyte2[j]=hd_temp2;
    //                hd_temp2=0;
    //                j=j+1;
    //                }

    //                qDebug() << "k=" <<k ;
                    kval=kval+1;


    //    }while(f.atEnd());
        }while(kval<=(fillen-3200-400)/(240+4*tlength)-1 );


        spxh.insert(0,spx.at(0));
        spyh.insert(0,spy.at(0));
        recxh.insert(0,recx.at(0));
        recyh.insert(0,recy.at(0));
    //      }while(k<=280);




//        qDebug() << "here";



        double maxelev = *std::max_element(recelev.begin(),recelev.end());
        double minelev = *std::min_element(recelev.begin(),recelev.end());

//        double binsize = 2*(recelev.at(recelev.length()/4)-recelev.at(recelev.length()*3./4.))*qPow(recelev.length(),-1./3.);
        double binsize = (1./2.)*(recelev.at(recelev.length()/4)-recelev.at(recelev.length()*3./4.))*qPow(recelev.length(),-1./3.);
        double numbins = ( maxelev - minelev )/ (binsize+0.000001);
        QVector<double> histogramplot(0);
        QVector<double> binplot(0);
        double histogram[qCeil(numbins)]={};

        double k=*std::min_element(recelev.begin(),recelev.end());
        int k1=0;
        if (*std::max_element(recelev.begin(),recelev.end())==0) {
            QMessageBox err_elev;
            err_elev.setIcon(QMessageBox::Warning);
            err_elev.setText("Null elevations on curent headers");
            err_elev.exec();}
        else{
        while (k <= *std::max_element(recelev.begin(),recelev.end())  ){
//            qDebug() << "here2" << k;
            for(int j=0;j<recelev.length();j++){
                if (recelev.at(j)>k && recelev.at(j)<k+binsize ){histogram[k1]=histogram[k1]+1;}
            }
            binplot.append((k+k+binsize)/2.);
            k=k+binsize;
            histogramplot.append(histogram[k1]);
            k1=k1+1;
        }}


        double maxbinval = *std::max_element(binplot.begin(),binplot.end())+binsize/2.;
        double minbinval = *std::min_element(binplot.begin(),binplot.end())-binsize/2.;





    //  Adding graph with the elevation profile
        QPen penelev;
        penelev.setWidth(1);
        penelev.setColor(QColor(0,0,255,255));
        QCPBars *hist = new QCPBars(ui->elevprof->yAxis,ui->elevprof->xAxis);
        ui->elevprof->yAxis->setLabel("Elevation (m)"); //adding labels
        ui->elevprof->xAxis->setLabel("Receivers");
        hist->setAntialiased(false);
        hist->setData(binplot,histogramplot);
        hist->setPen(penelev);
        hist->setWidth(binsize);
        ui->elevprof->yAxis->setRange(minbinval-1,maxbinval+1);
        ui->elevprof->xAxis->setRange(0,*std::max_element(histogramplot.begin(),histogramplot.end())+1);

        ui->elevprof->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        ui->elevprof->axisRect(0)->setRangeDrag(Qt::Horizontal);
        ui->elevprof->axisRect(0)->setRangeZoom(Qt::Horizontal);
        connect(ui->elevprof->xAxis,SIGNAL(rangeChanged(QCPRange)),this,SLOT(onhistXRangeChanged(QCPRange)));
//        ui->elevprof->rescaleAxes();
        QCPTextElement *title_hist = new QCPTextElement(ui->elevprof);
        title_hist->setText("Receiver elevation histogram");
        title_hist->setFont(QFont("sans", 10, QFont::Bold));
        ui->elevprof->plotLayout()->insertRow(0);
        ui->elevprof->plotLayout()->addElement(0,0,title_hist);


















    //  Adding graph with all the shots and the receivers


        QCPTextElement *title_basemap = new QCPTextElement(ui->mapplot);
        title_basemap->setText("Basemap");
        title_basemap->setFont(QFont("sans", 10, QFont::Bold));
        ui->mapplot->plotLayout()->insertRow(0);
        ui->mapplot->plotLayout()->addElement(0,0,title_basemap);

        QPen pen;
        pen.setWidth(1);
        pen.setColor(QColor(255,0,0,255));
        ui->mapplot->yAxis->setLabel("Northing (m)"); //adding labels
        ui->mapplot->xAxis->setLabel("Easting (m)");
        ui->mapplot->addGraph();
        ui->mapplot->graph(0)->setName("Shots");
        QCPScatterStyle scatterstylesp(QCPScatterStyle::ssCircle,QColor(255,0,0,255),QColor(255,0,0,0),7);
        ui->mapplot->graph(0)->setScatterStyle(scatterstylesp);
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
        ui->mapplot->graph(1)->setName("Receivers");
        QCPScatterStyle scatterstylerec(QCPScatterStyle::ssCircle,QColor(0,0,255,255),QColor(0,0,255,0),7);
        ui->mapplot->graph(1)->setScatterStyle(scatterstylerec);
        ui->mapplot->graph(1)->setLineStyle(QCPGraph::lsNone);
        ui->mapplot->graph(1)->setPen(recpen);
        ui->mapplot->graph(1)->addData(recx,recy);  //check QCPMapData
        ui->mapplot->rescaleAxes();



        QPen penh;
        penh.setWidth(1);
        penh.setColor(QColor(255,0,0,255));
        ui->mapplot->addGraph();
        ui->mapplot->graph(2)->setName("Current shot");
        QCPScatterStyle scatterstylesph(QCPScatterStyle::ssDisc,QColor(255,0,0,255),QColor(255,0,0,255),7);
        ui->mapplot->graph(2)->setScatterStyle(scatterstylesph);
        ui->mapplot->graph(2)->setLineStyle(QCPGraph::lsNone);
        ui->mapplot->graph(2)->setPen(penh);
        ui->mapplot->graph(2)->setData(spxh,spyh);  //check QCPMapData



        QPen recpenh;
        recpenh.setWidth(1);
        recpenh.setColor(QColor(0,0,0,255));
        ui->mapplot->addGraph();
        ui->mapplot->graph(3)->setName("Current receiver");
        QCPScatterStyle scatterstylerech(QCPScatterStyle::ssCircle,QColor(0,0,255,255),QColor(0,0,255,255),7);
        ui->mapplot->graph(3)->setScatterStyle(scatterstylerech);
        ui->mapplot->graph(3)->setLineStyle(QCPGraph::lsNone);
        ui->mapplot->graph(3)->setPen(recpenh);
        ui->mapplot->graph(3)->setData(recxh,recyh);  //check QCPMapData



        ui->mapplot->legend->setVisible(true);
}


void geom_ex::shotrechighlight(int trpos){
    QFile f(modelname);
    f.open(QFile::ReadOnly);
    f.seek(3200+400+(240+4*tlength)*(trpos-1));
    thead=f.read(240);

    int j=0;
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

    QVector<double> spxh(0),spyh(0);
    QVector<double> recxh(0),recyh(0);

    spxh.insert(0,tr_lbyte2[0].toHex().toInt(nullptr,16));
    spyh.insert(0,tr_lbyte2[1].toHex().toInt(nullptr,16));
    recxh.insert(0,tr_lbyte2[2].toHex().toInt(nullptr,16));
    recyh.insert(0,tr_lbyte2[3].toHex().toInt(nullptr,16));




    QPen pen;
    pen.setWidth(1);
    pen.setColor(QColor(0,0,0,255));
    ui->mapplot->graph(2)->setName("Current shot");
    QCPScatterStyle scatterstylesp(QCPScatterStyle::ssDisc,QColor(255,0,0,255),QColor(255,0,0,255),7);
    ui->mapplot->graph(2)->setScatterStyle(scatterstylesp);
    ui->mapplot->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->mapplot->graph(2)->setPen(pen);
    ui->mapplot->graph(2)->setData(spxh,spyh);  //check QCPMapData



    QPen recpen;
    recpen.setWidth(1);
    recpen.setColor(QColor(0,0,0,255));
    ui->mapplot->graph(3)->setName("Current receiver");
    QCPScatterStyle scatterstylerec(QCPScatterStyle::ssCircle,QColor(0,0,255,255),QColor(0,0,255,255),7);
    ui->mapplot->graph(3)->setScatterStyle(scatterstylerec);
    ui->mapplot->graph(3)->setLineStyle(QCPGraph::lsNone);
    ui->mapplot->graph(3)->setPen(recpen);
    ui->mapplot->graph(3)->setData(recxh,recyh);  //check QCPMapData

    ui->mapplot->replot();




}


void geom_ex::slotMouseDoubleClick(QMouseEvent *ev){

        ui->mapplot->removeGraph(4);
        ui->mapplot->removeItem(0);
        ui->mapplot->replot();
        numpoints=0;

}

void geom_ex::slotMouseClick(QMouseEvent *ev){

    double xc = ui->mapplot->xAxis->pixelToCoord(ev->pos().x());
    double yc = ui->mapplot->yAxis->pixelToCoord(ev->pos().y());

    if(numpoints==0) {
        QPen marker;
        marker.setWidth(1);
        marker.setColor(QColor(0,0,0,255));
        ui->mapplot->addGraph();
        ui->mapplot->graph(4)->removeFromLegend();
        QCPScatterStyle markerstyle(QCPScatterStyle::ssPlus,QColor(0,0,0,255), QColor(0,0,0,255),7);
        ui->mapplot->graph(4)->setScatterStyle(markerstyle);
        ui->mapplot->graph(4)->addData(xc,yc);
        ui->mapplot->removeItem(0);
        ui->mapplot->replot();
        numpoints=numpoints+1;
        xi=xc;
        yi=yc;
    }
    else if(numpoints==1) {
        ui->mapplot->graph(4)->addData(xc,yc);
//        qDebug()<< "second marker" << numpoints << ui->mapplot->graphCount();
        xe=xc;
        ye=yc;
        xm=(xe+xi)/2.0;
        ym=(ye+yi)/2.0;
        double dist=qSqrt((xe-xi)*(xe-xi)+(ye-yi)*(ye-yi));
        numpoints=numpoints+1;
        QCPItemText *textLabel = new QCPItemText(ui->mapplot);
//        ui->mapplot->addItem(textLabel);
        QBrush brush(Qt::white,Qt::SolidPattern);
        //add here conditional if for meter of feet display on map
        if(unit==1){textLabel->setText(QString::number(dist,'f',2)+" m");}
        else if(unit==2){textLabel->setText(QString::number(dist,'f',2)+" f");}
        else{textLabel->setText("Non-valid measurement units");};
//        textLabel->setText(QString::number(dist,'f',2)+" m");
        textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
        textLabel->setPen(QPen(Qt::black)); // show black border around text
        textLabel->setBrush(brush);
        textLabel->position->setCoords(xm,ym);
        ui->mapplot->replot();
    }
//    else if(numpoints==2){
//        ui->mapplot->removeGraph(4);
//        ui->mapplot->removeItem(0);
//        ui->mapplot->replot();
//        numpoints=0;
//    }

}







void geom_ex::slotMouseMove(QMouseEvent *ev){
    double xc = ui->mapplot->xAxis->pixelToCoord(ev->pos().x());
    double yc = ui->mapplot->yAxis->pixelToCoord(ev->pos().y());

//    ui->xcoord->setText("X="+QString::number(xc,10));
//    ui->ycoord->setText("Y="+QString::number(yc,10));
    ui->xcoord->setText("X="+QString::number(xc,'f',2));
    ui->ycoord->setText("Y="+QString::number(yc,'f',2));
//    qDebug() << ui->mapplot->xAxis->pixelToCoord(ev->pos().x()) << ui->mapplot->yAxis->pixelToCoord(ev->pos().y());
}


geom_ex::~geom_ex()
{
    delete ui;
}



void geom_ex::on_measure_toggled(bool checked)
{
//    qDebug() << ui->mapplot->graphCount();
    if(checked==true){
     connect(ui->mapplot,&QCustomPlot::mousePress, this, &geom_ex::slotMouseClick);
     connect(ui->mapplot,&QCustomPlot::mouseDoubleClick, this, &geom_ex::slotMouseDoubleClick);
    }
    else{
     disconnect(ui->mapplot,&QCustomPlot::mousePress, this, &geom_ex::slotMouseClick);
     disconnect(ui->mapplot,&QCustomPlot::mouseDoubleClick, this, &geom_ex::slotMouseDoubleClick);
     ui->mapplot->removeGraph(4);
     ui->mapplot->removeItem(0);
     numpoints=0;
     ui->mapplot->replot();
    }

}






void geom_ex::onhistXRangeChanged(const QCPRange &range)
{
    QCPRange boundedRange = range;
    double lowerRangeBound = 0;
    double upperRangeBound = 1.e6;



    if(boundedRange.lower < lowerRangeBound || qFuzzyCompare(boundedRange.size(),upperRangeBound-lowerRangeBound)) {  // restrict max zoom in
        boundedRange.lower = lowerRangeBound;
        boundedRange.upper = lowerRangeBound + boundedRange.size();
        if(boundedRange.upper > upperRangeBound){
            boundedRange.upper = upperRangeBound;
            ui->elevprof->xAxis->setRange(boundedRange);
        }
   }


    ui->elevprof->xAxis->setRange(boundedRange);
    ui->elevprof->replot();

}
