#ifndef GEOM_EX_H
#define GEOM_EX_H

#include <QWidget>
//#include <QtCharts/QLineSeries>
//#include <QChartView>
#include <QWheelEvent>
#include <QGestureEvent>
#include <QGesture>
//#include <QScatterSeries>
#include <QGestureEvent>
#include <qcustomplot.h>


//using namespace QtCharts;

namespace Ui {
class geom_ex;
}

class geom_ex : public QWidget
{
    Q_OBJECT

public:
    explicit geom_ex(QWidget *parent = 0);
    ~geom_ex();
//    QScatterSeries *recs = new QScatterSeries();
//    QScatterSeries *shots = new QScatterSeries();
    double markersize = 3;


protected:


private slots:
//    void mousePressEvent(QMouseEvent *ev);
    void slotMousePress(QMouseEvent *ev);
    void slotMouseMove(QMouseEvent *ev);


private:
    Ui::geom_ex *ui;



};

#endif // GEOM_EX_H
