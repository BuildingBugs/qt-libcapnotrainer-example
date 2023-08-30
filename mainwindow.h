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

    // add references to Label and ProgressBar
    QLabel *statusLabel;
    QLabel *petCO2Label;
    QLabel *batteryLabel;
    QLabel *bpmLabel;

    void userCapnoCallback(std::vector<float> data, DeviceType device_type, uint8_t conn_handle, DataType data_type);
    void startBlockingFunction(void);
    CapnoTrainer capnoTrainer;

    // some variables that should be part of subclasses
    uint32_t co2Samples = 0;
    uint32_t co2DataDownsample = 1;
    double co2Rate = 100.0;

    // this needs to be thread-safe (as shared resource).
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





#endif // MAINWINDOW_H
