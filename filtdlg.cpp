#include "filtdlg.h"
#include "ui_filtdlg.h"
#include "trace_group_ex.h"
#include <QDebug>
#include <QMessageBox>

extern int grp_trpos;
extern double grp_gain;
extern double grp_clip;
extern int grp_numtr;
extern int grp_filttype;

double lowCutFreq;
double lowCutSlope;
double highCutFreq;
double highCutSlope;
double lowPassFreq;
double highPassFreq;


filtdlg::filtdlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::filtdlg)
{
    ui->setupUi(this);
    ui->radioButton->setChecked(true);
}

filtdlg::~filtdlg()
{
    delete ui;
}

void filtdlg::on_checkBox_toggled(bool checked)
{
    ui->scrollArea->setEnabled(checked);

}

void filtdlg::on_radioButton_toggled(bool checked)
{
    ui->comboBox->setEnabled(checked);
    ui->groupBox->setEnabled(checked);
    ui->comboBox->setCurrentIndex(0);
    ui->freqCut2->setEnabled(false);
    ui->freqSlope2->setEnabled(false);
}

void filtdlg::on_radioButton_2_toggled(bool checked)
{
    ui->comboBox_2->setEnabled(checked);
    ui->groupBox_2->setEnabled(checked);
    ui->comboBox_2->setCurrentIndex(0);
}

void filtdlg::on_pushButton_clicked()
{
    int numfilt1 = ui->comboBox->count();
    int numfilt2 = ui->comboBox_2->count();

    if (ui->checkBox->isChecked()&&ui->radioButton->isChecked()) {grp_filttype=1+ui->comboBox->currentIndex();
        switch (grp_filttype) {
        case 1:
            lowCutFreq=ui->freqCut1->value();
            lowCutSlope=ui->freqSlope1->value();
            break;
        case 2:
            highCutFreq=ui->freqCut2->value();
            highCutSlope=ui->freqSlope2->value();
            break;
        case 3:
            lowCutFreq=ui->freqCut1->value();
            lowCutSlope=ui->freqSlope1->value();
            highCutFreq=ui->freqCut2->value();
            highCutSlope=ui->freqSlope2->value();
            if(highCutFreq>lowCutFreq){
                QMessageBox msgbox;
                msgbox.warning(0,"Warning","High pass frequency cut-off must be smaller than low pass frequency cut-off");
                grp_filttype=0;
            }
            break;
        default:
            break;
        }


    }
    else if (ui->checkBox->isChecked()&&ui->radioButton_2->isChecked()) {grp_filttype=1+numfilt1+ui->comboBox_2->currentIndex();
          lowCutFreq=ui->freqCutLow->value();
          lowPassFreq=ui->freqPassLow->value();
          highPassFreq=ui->freqPassHigh->value();
          highCutFreq=ui->freqCutHigh->value();
          if(highPassFreq<lowPassFreq){
              QMessageBox msgbox;
              msgbox.warning(0,"Warning","High pass frequency must be greater than low pass frequency");
              grp_filttype=0;
          }
          else if (highCutFreq<lowCutFreq) {
              QMessageBox msgbox;
              msgbox.warning(0,"Warning","High cut frequency must be greater than low cut frequency");
              grp_filttype=0;
          }
          else if (lowCutFreq>lowPassFreq) {
              QMessageBox msgbox;
              msgbox.warning(0,"Warning","Low cut frequency must be smaller than low pass frequency");
              grp_filttype=0;
          }
          else if (highCutFreq<highPassFreq) {
              QMessageBox msgbox;
              msgbox.warning(0,"Warning","High cut frequency must be greater than high pass frequency");
              grp_filttype=0;
          }
          else if (highPassFreq<lowCutFreq) {
              QMessageBox msgbox;
              msgbox.warning(0,"Warning","High pass frequency must be greater than low cut frequency");
              grp_filttype=0;
          }
          else if (highCutFreq<lowPassFreq) {
              QMessageBox msgbox;
              msgbox.warning(0,"Warning","High cut frequency must be greater than low pass frequency");
              grp_filttype=0;
          }

    }
    else {grp_filttype=0;}





    emit applyParams(grp_trpos,grp_numtr,grp_gain,grp_clip,grp_filttype);
}

void filtdlg::on_comboBox_currentIndexChanged(int index)
{

    switch (index) {
    case 0:
        ui->freqCut1->setEnabled(true);
        ui->freqSlope1->setEnabled(true);
        ui->freqCut2->setEnabled(false);
        ui->freqSlope2->setEnabled(false);
        break;
    case 1:
        ui->freqCut2->setEnabled(true);
        ui->freqSlope2->setEnabled(true);
        ui->freqCut1->setEnabled(false);
        ui->freqSlope1->setEnabled(false);
        break;
    case 2:
        ui->freqCut1->setEnabled(true);
        ui->freqSlope1->setEnabled(true);
        ui->freqCut2->setEnabled(true);
        ui->freqSlope2->setEnabled(true);
        break;
    default:
        break;
    }
//    if(index==0){ui->freqCut1->setEnabled(true);ui->freqSlope1->setEnabled(true);ui->freqCut2->setEnabled(false);ui->freqSlope2->setEnabled(false);}
//    else if(index==1){ui->freqCut2->setEnabled(true);ui->freqSlope2->setEnabled(true);ui->freqCut1->setEnabled(false);ui->freqSlope1->setEnabled(false);}
//    else if(index==2){ui->freqCut1->setEnabled(true);ui->freqSlope1->setEnabled(true);ui->freqCut2->setEnabled(true);ui->freqSlope2->setEnabled(true);}
}




void filtdlg::on_pushButton_2_clicked()
{
    this->close();
}
