#include "paramdialog.h"
#include "ui_paramdialog.h"
#include "trace_group_ex.h"
#include <QDebug>




extern int grp_trpos;
extern double grp_gain;
extern double grp_clip;
extern int grp_numtr;
extern quint32 grp_fillen;
extern quint32 tlength;
extern int grp_filttype;



paramdialog::paramdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::paramdialog)
{
    ui->setupUi(this);

    ui->spinBox->setMinimum(1);
    ui->spinBox_2->setMinimum(1);
    ui->spinBox_2->setMaximum(1000);
    ui->spinBox_3->setMinimum(0);
//    connect(ui->spinBox,SIGNAL(editingFinished()),this,SLOT(on_editFin()));
//    connect(ui->spinBox_2,SIGNAL(editingFinished()),this,SLOT(on_editFin()));
//    connect(ui->spinBox_3,SIGNAL(editingFinished()),this,SLOT(on_editFin()));
//    connect(ui->spinBox_4,SIGNAL(editingFinished()),this,SLOT(on_editFin()));


}

paramdialog::~paramdialog()
{
    delete ui;
}

void paramdialog::on_cancelButton_clicked()
{
    this->close();
}


void paramdialog::on_editFin()
{
//    grp_trpos=ui->spinBox->value();
//    grp_numtr=ui->spinBox_2->value();
//    grp_gain=ui->spinBox_3->value();
//    grp_clip=ui->spinBox_4->value();

//    if(grp_trpos>(grp_fillen-3200-400)/(240+4*tlength)){
//        grp_trpos=(grp_fillen-3200-400)/(240+4*tlength);
//        ui->spinBox->setValue(grp_trpos);
//    }

//    if (grp_numtr + grp_trpos > (grp_fillen-3200-400)/(240+4*tlength)+1) {
//        grp_numtr = (grp_fillen-3200-400)/(240+4*tlength)-grp_trpos+1;
//        ui->spinBox_2->setValue(grp_numtr);
//    }
//    emit applyParams(grp_trpos,grp_numtr,grp_gain,grp_clip);

}


void paramdialog::on_applyButton_clicked()
{

    grp_trpos=ui->spinBox->value();
    grp_numtr=ui->spinBox_2->value();
    grp_gain=ui->spinBox_3->value();
    grp_clip=ui->spinBox_4->value();

    if(grp_trpos>(grp_fillen-3200-400)/(240+4*tlength)){
        grp_trpos=(grp_fillen-3200-400)/(240+4*tlength);
        ui->spinBox->setValue(grp_trpos);
    }

    if (grp_numtr + grp_trpos > (grp_fillen-3200-400)/(240+4*tlength)+1) {
        grp_numtr = (grp_fillen-3200-400)/(240+4*tlength)-grp_trpos+1;
        ui->spinBox_2->setValue(grp_numtr);
    }

    emit applyParams(grp_trpos,grp_numtr,grp_gain,grp_clip,grp_filttype);
//    this->close();


}
