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
//#include <geom_ex.h>
class geom_ex;

namespace Ui {
class trace_ex;
}

class trace_ex : public QWidget
{
    Q_OBJECT

public:
    explicit trace_ex(QWidget *parent = 0);
    ~trace_ex();
    friend class geom_ex;

private:
    Ui::trace_ex *ui;
    geom_ex *geomwin;


public slots:
     void traceread(int trpos, bool smooth, bool rescale);
     void tracespecplot(QVector<double> &trace, int N, QVector<double> &freq, QVector<double> &time, std::complex<double> sig[], QVector<double> &possig, QVector<double> &traceposi, QVector<double> &timeposi, bool rescale);
     void wvplot(QVector<double> &trace, int N, QVector<double> &freq, QVector<double> &time, bool smooth, bool rescale);
     void onYRangeChanged(const QCPRange &range);
     void onspecXRangeChanged(const QCPRange &range);



private slots:
     void slotMouseMove(QMouseEvent *ev);


     void on_checkBox_toggled(bool checked);

     void on_horizontalSlider_valueChanged(int value);

     void on_tableWidgettr_cellClicked(int row, int column);

     void on_toolButton_clicked();

     void on_tableWidgettr_cellChanged(int row, int column);


signals:

    wvd_signal(int,bool,bool);
    reloadTrace(int, bool, bool);


};

#endif // TRACE_EX_H
