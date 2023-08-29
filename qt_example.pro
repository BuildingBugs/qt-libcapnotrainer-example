#-------------------------------------------------
#
# Project created by QtCreator 2023-07-30T23:57:44
#
#-------------------------------------------------

QT       += core gui serialport
QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt_example
TEMPLATE = app



SOURCES += main.cpp\
        mainwindow.cpp \
        qcustomplot.cpp


HEADERS  += mainwindow.h \
        qcustomplot.h

FORMS    += mainwindow.ui



win32: LIBS += -L$$PWD/third_party/capnotrainer/lib/ -llibcapnotrainergo

INCLUDEPATH += $$PWD/third_party/asio/asio/include
INCLUDEPATH += $$PWD/third_party/capnotrainer/include

DEPENDPATH += $$PWD/third_party/capnotrainer/lib
DEPENDPATH += $$PWD/third_party/capnotrainer/include

