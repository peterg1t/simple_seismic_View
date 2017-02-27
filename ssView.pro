#-------------------------------------------------
#
# Project created by QtCreator 2016-04-26T18:35:07
#
#-------------------------------------------------

QT       += core gui quickwidgets location positioning network charts widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport location

TARGET = ssView
TEMPLATE = app


SOURCES +=\
        mainwindow.cpp \
    trace_ex.cpp \
    trace_group_ex.cpp \
    qcustomplot.cpp \
    main.cpp \
    fft_complx.cpp \
    geom_ex.cpp \
    hilbert.cpp \
    wignerville.cpp


HEADERS  += mainwindow.h \
    trace_ex.h \
    trace_group_ex.h \
    qcustomplot.h \
    fft_complx.h \
    geom_ex.h \
    C:/Qt/5.7/mingw53_32/include/QtCharts/qchartview.h \
    hilbert.h \
    wignerville.h

FORMS    += mainwindow.ui \
    trace_ex.ui \
    trace_group_ex.ui \
    geom_ex.ui

DISTFILES += \
    geom_map.qml
