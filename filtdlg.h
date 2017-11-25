#ifndef FILTDLG_H
#define FILTDLG_H

#include <QDialog>

namespace Ui {
class filtdlg;
}

class filtdlg : public QDialog
{
    Q_OBJECT

public:
    explicit filtdlg(QWidget *parent = 0);
    ~filtdlg();

private slots:
    void on_checkBox_toggled(bool checked);

    void on_radioButton_toggled(bool checked);

    void on_radioButton_2_toggled(bool checked);

    void on_pushButton_clicked();


    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_2_clicked();


signals:
    void applyParams(int,int,double,double,int);



private:
    Ui::filtdlg *ui;
};

#endif // FILTDLG_H
