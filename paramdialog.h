#ifndef PARAMDIALOG_H
#define PARAMDIALOG_H

#include <QDialog>


namespace Ui {
class paramdialog;
}

class paramdialog : public QDialog
{
    Q_OBJECT

public:
    explicit paramdialog(QWidget *parent = 0);
    ~paramdialog();
    friend class trace_group_ex;


public slots:


private slots:
    void on_cancelButton_clicked();



    void on_applyButton_clicked();

    void on_editFin();

signals:
    void applyParams(int,int,double,double,int);


private:
    Ui::paramdialog *ui;    



};

#endif // PARAMDIALOG_H
