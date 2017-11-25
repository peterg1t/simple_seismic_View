#include "fbdlg.h"
#include "ui_fbdlg.h"
#include <QDebug>
#include <math.h>


//extern QVector<double> trace;
extern int grp_trpos;
extern int grp_numtr;
extern int grp_fbtype;
int shtTrmWnd; // short term window size
int EPSWndSz;  // EPS window size
double Beta;      // FB stability constant
int fbwStart;
int fbwEnd;
extern int intsample;
double fbThres;





fbdlg::fbdlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::fbdlg)
{
    ui->setupUi(this);
}

fbdlg::~fbdlg()
{
    delete ui;
}

void fbdlg::on_pushButton_2_clicked()
{
    this->close();
}

void fbdlg::on_pushButton_clicked()
{    
    shtTrmWnd=ui->doubleSpinBox->value();
    EPSWndSz=ui->doubleSpinBox_2->value();
    Beta=ui->doubleSpinBox_3->value();
    fbwStart=round(ui->doubleSpinBox_4->value()/(intsample*1e-3));
    fbwEnd=round(ui->doubleSpinBox_5->value()/(intsample*1e-3));
    fbThres=ui->doubleSpinBox_6->value();
    emit applyfbParams(grp_trpos,grp_numtr,grp_fbtype);
}


void fbdlg::on_radioButton_toggled(bool checked)
{
    ui->groupBox->setEnabled(checked);

}

void fbdlg::on_doubleSpinBox_2_valueChanged(double arg1)
{
    if(fmod(arg1,2)==0){arg1=arg1+1;}
    ui->doubleSpinBox_2->setValue(arg1);
}
