#ifndef TRACE_GROUP_EX_H
#define TRACE_GROUP_EX_H

#include <QWidget>
#include <qcustomplot.h>
#include "paramdialog.h"

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
    paramdialog *paramsdlg;



public slots:

     void onYRangeChanged(const QCPRange &range);


private slots:
     void grp_traceread(int grp_trpos, int grp_numtr, double grp_gain, double grp_clip);
     void slotMouseMove(QMouseEvent *ev);
     void on_params_clicked();
     void on_fitall_clicked();
     void on_fitvert_clicked();
     void on_fithor_clicked();
};

#endif // TRACE_GROUP_EX_H
