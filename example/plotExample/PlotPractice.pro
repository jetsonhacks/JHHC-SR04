#-------------------------------------------------
#
# Project created by QtCreator 2015-05-06T19:05:34
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = PlotPractice
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    jetsonGPIO.c \
    hcsr04.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    jetsonGPIO.h \
    hcsr04.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    Border 1920x1080.png \
    Border 1920x1080.png \
    Border 1920x1080.png

RESOURCES += \
    PlotPractice.qrc
