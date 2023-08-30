#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <queue>
#include <vector>

#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <QTimer>

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

    void userCapnoCallback(std::vector<float> data, DeviceType device_type, uint8_t conn_handle, DataType data_type);
    void startBlockingFunction(void);
    CapnoTrainer capnoTrainer;

    // some variables that should be part of subclasses
    uint32_t co2Samples = 0;
    uint32_t co2DataDownsample = 5;
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



class CapnoWorker : public QObject {
    Q_OBJECT

private:
    CapnoTrainer* capno;

public:
    CapnoWorker(CapnoTrainer* capnoInstance) : capno(capnoInstance) {}

public slots:
    void doWork() {
        capno->Initialize();
        emit finished();
    }

signals:
    void finished();
};



#endif // MAINWINDOW_H
