#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <unistd.h>

#include "jetsonGPIO.h"
#include "hcsr04.h"

using namespace std;

HCSR04 *hcsr04 ;



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // setStyleSheet(" MainWindow {background-image: url(:/images/BorderImage) ;background-color: black;}");
    setStyleSheet(" MainWindow {background-color: black;}");
    // setStyleSheet(" MainWindow {background-image:  url(:/images/Wallpaper), url(:/images/BorderImage);}");

    ui->distanceLabel->setStyleSheet("QLabel { background-color : black; color : white; }");

    ui->consoleLog->setMaximumBlockCount(14) ;
    ui->consoleLog->setCenterOnScroll(false) ;

    hcsr04 = new HCSR04() ;
    hcsr04->exportGPIO();
    hcsr04->setDirection();

    setupDistancePlot(ui->distancePlot);
}

MainWindow::~MainWindow()
{
    delete ui;
    hcsr04->unexportGPIO();
}

void MainWindow::setupDistancePlot(QCustomPlot *customPlot) {
    customPlot->addGraph() ;
    //QCPBars *bars1 = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    //customPlot->addPlottable(bars1);
    //bars1->setWidth(6);
    // bars1->setData(x3, y3);
    //bars1->setPen(Qt::NoPen);
    //bars1->setBrush(QColor(10, 140, 70, 160));
    customPlot->setBackground(Qt::lightGray);
    QPen greenPen = QPen(QColor(0, 128, 0, 200)) ;
    greenPen.setWidth(3) ;
    // velocityPlot->graph(0)->setPen(QPen(Qt::blue));
    customPlot->graph(0)->setPen(greenPen);

    customPlot->graph(0)->setBrush(QBrush(QColor(10, 140, 70, 160)));
    customPlot->graph(0)->setAntialiasedFill(true);



    // Setup X Axis
    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
    customPlot->xAxis->setAutoTickStep(false);
    customPlot->xAxis->setTickStep(2);
    customPlot->axisRect()->setupFullAxesBox();

    // customPlot->yAxis->setRange(0,1000) ;
    customPlot->yAxis->setRange(0,500) ;
    customPlot->yAxis->setLabel("Distance") ;

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::distanceDataSlot:
    connect(&distanceDataTimer, SIGNAL(timeout()), this, SLOT(distanceDataSlot()));
    distanceDataTimer.start(0); // Interval 0 means to refresh as fast as possible

}

void MainWindow::distanceDataSlot() {
    // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    double key = 0;
#else
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.01) // at most add point every 10 ms
    {
        int distance = hcsr04->pingMedian(5) / 58 ;
        char resultBuffer[128] ;
        // sprintf(resultBuffer," Distance: %5dcm Previous Distance: %5dcm Velocity: %08d",distance,previousDistance,velocity);
        sprintf(resultBuffer," Distance: %5dcm ",distance);
        ui->consoleLog->appendPlainText(resultBuffer);
        ui->consoleLog->ensureCursorVisible();


        int value0 = distance ;
        if (value0 != -1) {
            // add data to bars:
            ui->distancePlot->graph(0)->addData(key, value0);
        }
        // set data of dots:
        // remove data of lines that's outside visible range:
        ui->distancePlot->graph(0)->removeDataBefore(key-8);
        // rescale value (vertical) axis to fit the current data:
        // ui->velocityPlot->graph(0)->rescaleValueAxis();
        lastPointKey = key;

        // Set the distance label
        char buffer[32] ;
        sprintf(buffer,"%5d cm",distance) ;
        ui->distanceLabel->setText(buffer);
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->distancePlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
    ui->distancePlot->replot();
}

