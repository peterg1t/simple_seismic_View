#ifndef TRACE_GROUP_EX_H
#define TRACE_GROUP_EX_H

#include <QWidget>
#include <qcustomplot.h>

namespace Ui {
class trace_group_ex;
}

class trace_group_ex : public QWidget
{
    Q_OBJECT

public:
    explicit trace_group_ex(QWidget *parent = 0);
    ~trace_group_ex();

private:
    Ui::trace_group_ex *ui;




public slots:
     void traceread(int trpos, int grp_numtr);
     void onYRangeChanged(const QCPRange &range);


private slots:
     void on_pushButton_clicked();
     void on_spinBox_3_editingFinished();
};

#endif // TRACE_GROUP_EX_H
