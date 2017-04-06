#ifndef TRACE_EX_H
#define TRACE_EX_H

#include <QWidget>
#include <qcustomplot.h>
#include <QtCharts>
#include <QSplineSeries>
#include <complex>
#include <QVector>
#include <QGesture>
#include <QGestureEvent>
#include <QWheelEvent>

namespace Ui {
class trace_ex;
}

class trace_ex : public QWidget
{
    Q_OBJECT

public:
    explicit trace_ex(QWidget *parent = 0);
    ~trace_ex();

private:
    Ui::trace_ex *ui;    




public slots:
     void traceread(int trpos);
     void onYRangeChanged(const QCPRange &range);
     void onspecXRangeChanged(const QCPRange &range);



private slots:
     void slotMouseMove(QMouseEvent *ev);





};

#endif // TRACE_EX_H
