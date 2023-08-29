#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
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
    CapnoTrainer *capnoTrainer = nullptr;

    // some variables that should be part of subclasses
    uint32_t co2Samples = 0;
    double co2Rate = 100.0;



private slots:
    void onButtonClicked();
    void updateGraph(std::vector<float>);


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
