#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "trace_ex.h"
#include "geom_ex.h"
#include "trace_group_ex.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();



private slots:
    void on_actionExit_triggered();

    void on_radioButton_toggled(bool checked);



    void on_radioButton_2_toggled(bool checked);

    void on_actionAbout_triggered();

    void on_actionCreate_geometry_from_model_triggered();

    void on_actionExplore_SEGY_by_groups_triggered();

    void on_actionExplore_seq_triggered();


private:
    Ui::MainWindow *ui; //declaring ui of main windows
    trace_ex *tracewin; // declaring ui of windows trace_ex
    geom_ex *geomwin; // declaring ui of windows trace_ex
    trace_group_ex *tracewingrp; // declaring ui of windows trace_group_ex




    //    this allows to close all the windows of the progam theother part to this is in mainwindow.cpp
    void closeEvent(QCloseEvent *event);


};



#endif // MAINWINDOW_H
