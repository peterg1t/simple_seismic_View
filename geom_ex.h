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
#include <trace_ex.h>


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
    friend class trace_ex;



protected:

public slots:
    void onhistXRangeChanged(const QCPRange &range);



private slots:
//    void mousePressEvent(QMouseEvent *ev);
    void slotMouseClick(QMouseEvent *ev);
    void slotMouseDoubleClick(QMouseEvent *ev);
    void slotMouseMove(QMouseEvent *ev);
    void shotrechighlight(int trpos);
    void on_measure_toggled(bool checked);
    void mapread(int bi[], int bl[]);



private:
    Ui::geom_ex *ui;



};

#endif // GEOM_EX_H
