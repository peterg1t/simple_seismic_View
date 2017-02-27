#include "mainwindow.h"
#include <QApplication>
#include <string>




int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("ssView");
    w.show();

    return a.exec();
}
