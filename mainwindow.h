#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <queue>
#include <vector>
#include <algorithm>

#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QLabel>


#include "commons.h"
#include "capnotrainer.h"
#include "qcustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private:

    Ui::MainWindow *ui;

    // some statusbar labels for easy debugging.
    QLabel *statusLabel;
    QLabel *petCO2Label;
    QLabel *insCO2Label;
    QLabel *batteryLabel;
    QLabel *bpmLabel;

    void userCapnoCallback(std::vector<float> data, DeviceType device_type, uint8_t conn_handle, DataType data_type);
    void startBlockingFunction(void);
    CapnoTrainer capnoTrainer;

    // some variables that should be part of struct.

    // use to calculate the x-axis time point for each sample.
    uint32_t co2Samples = 0;
    // increase this as if you want to keep more data on graph.
    // for 1 minute with 100 samples/seconds, you'll have
    // 6000 points.
    uint32_t co2DataDownsample = 1;
    double co2Rate = 100.0; // sample rate is almost 100.

    // this needs to be thread-safe (as shared resource).
    // you can make the similar one for HRV (rr-interval and hr)
    // or emgs 1 - 4 channels (see user_callback).
    std::queue<std::vector<float>> co2Queue;

    // graph timer
    QTimer graphPlotTimer;
    QMutex dataMutex;
    int maxQueueSize = 100;


private slots:
    void onConnectBtnClicked();
    void onClearGraphBtnClicked();
    void updateGraph();
};



class TimeAxisTicker : public QCPAxisTicker
{
public:
    QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override
    {
        Q_UNUSED(locale)
        Q_UNUSED(formatChar)
        Q_UNUSED(precision)

        int totalSeconds = static_cast<int>(tick);
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;

        return tick >= 0 ? QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')) : QString("");
    }
};


#endif // MAINWINDOW_H
