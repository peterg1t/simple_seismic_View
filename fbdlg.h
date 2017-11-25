#ifndef FBDLG_H
#define FBDLG_H

#include <QDialog>

namespace Ui {
class fbdlg;
}

class fbdlg : public QDialog
{
    Q_OBJECT

public:
    explicit fbdlg(QWidget *parent = 0);
    ~fbdlg();
    friend class trace_group_ex;

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_radioButton_toggled(bool checked);

    void on_doubleSpinBox_2_valueChanged(double arg1);

signals:
    void applyfbParams(int,int,int);



private:
    Ui::fbdlg *ui;
};

#endif // FBDLG_H
