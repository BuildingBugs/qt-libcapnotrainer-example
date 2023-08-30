#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <cmath>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    capnoTrainer(std::bind(&MainWindow::userCapnoCallback, this,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          std::placeholders::_3,
                          std::placeholders::_4),
                 true
          )
{
    ui->setupUi(this);

    // print out capnoTrainer version
    std::cout << "CapnoTrainer library: " << CapnoTrainer::GetVersion() << std::endl;

    // Enumerate COM ports
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &info : availablePorts) {
        QString portName = info.portName();
        // probably add pid and vid filter here.
        ui->comboBox1->addItem(portName);
        ui->comboBox2->addItem(portName);
    }

    // Connect button signal to slot
    connect(ui->connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectBtnClicked);
    connect(ui->clearGraphBtn, &QPushButton::clicked, this, &MainWindow::onClearGraphBtnClicked);

    // setup graph
    ui->customPlot->addGraph(); // Add a graph
    ui->customPlot->graph(0)->setPen(QPen(Qt::green)); // Line color blue for first graph
    ui->customPlot->xAxis->setLabel("Time (in sec.)");
    ui->customPlot->yAxis->setLabel("Raw PCO2 (in mmHg)");
    ui->customPlot->yAxis->setRange(0, 1500); // Assuming you want a range from 0 to 100

    // setup graph timer (10Hz is perceived as real time).
    connect(&graphPlotTimer, &QTimer::timeout, this, &MainWindow::updateGraph);
    graphPlotTimer.start(100);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateGraph()
{
    // calculate two new data points:
    QVector<double> xData;
    QVector<double> yData;
    QMutexLocker locker(&dataMutex);

    while (!co2Queue.empty())
    {
        std::vector<float> data = co2Queue.back();
        co2Queue.pop();

        // downsampling the data due to high sampling rate.
        for (size_t i = 0; i < data.size(); i+=co2DataDownsample )
        {
            xData.push_back(((double)co2Samples / (co2Rate/co2DataDownsample)) );
            yData.push_back((double) data.at( i ));
            co2Samples += 1;
        }
    }

    // in case the queue is already empty (when the GO is not turned on).
    if (xData.size() > 0){

        double max_time = 30 ; // in seconds
        ui->customPlot->graph(0)->addData(xData, yData);
        // make key axis range scroll with the data (at a constant range size of 8):
        ui->customPlot->xAxis->setRange( xData.at(xData.size()-1) +0.25, max_time, Qt::AlignRight);
        //ui->customPlot->yAxis->setRange(550, 850);
        ui->customPlot->graph(0)->rescaleValueAxis();
        ui->customPlot->graph(0)->rescaleValueAxis(true);
        ui->customPlot->graph(0)->data()->removeBefore( xData.at(xData.size()-1) - max_time );
        ui->customPlot->replot();
    }
}


void MainWindow::userCapnoCallback(std::vector<float> data, DeviceType device_type, uint8_t conn_handle, DataType data_type)
{
    QMutexLocker locker(&dataMutex);

    switch (device_type)
    {
        case DONGLE_DEVTYPE_CAPNO_GO:
        {
            if (data_type == DATA_CO2)
            {

            }
            if (data_type == DATA_CAPNO_BATTERY)
            {
                std::cout << "Received Battery data with length: " << data.size() << "  with handle: " << (int)conn_handle << std::endl;
            }

            if (data_type == DATA_CAPNO_STATUS)
            {
                // NOT IMPLEMENTED
            }
        }
        break;

        case DONGLE_DEVTYPE_EMG:
        {
            if (data_type == DATA_EMG)
            {
                co2Queue.push(data);
                std::cout << "Received EMG data with length: " << data.size() << "  with handle: " << (int)conn_handle << std::endl;
            }
        }
        break;

        case DONGLE_DEVTYPE_HRV:
        {
            if (data_type == DATA_RR_INTERVALS)
            {
                std::cout << "Received RR-interval data with length: " << data.at(0) << "  with handle: " << (int)conn_handle << std::endl;
            }
            if (data_type == DATA_HEART_RATE)
            {
                std::cout << "Heart Rate" << std::endl;
            }
        }
        break;

        case DONGLE_DEVTYPE_CAPNO_6:
        {
            // NOT IMPLEMENTED
        }
        break;

        default:
            break;
    }

}

void MainWindow::onClearGraphBtnClicked(){

       std::cout << ui->customPlot->graph(0)->dataCount() << std::endl;
       ui->customPlot->graph(0)->data()->clear();
       ui->customPlot->replot();
}

void MainWindow::onConnectBtnClicked() {
    QString port1 = ui->comboBox1->currentText();
    QString port2 = ui->comboBox2->currentText();

    // Check if any of the ports is not selected
    if (port1.isEmpty() || port2.isEmpty()) {
        // Show a warning message
        QMessageBox::warning(this, "Warning", "Please select two COM ports before proceeding.");
        return;
    }

    if ( capnoTrainer.isConnected() )
    {
        try {
            capnoTrainer.Disconnect();
        } catch(const std::exception &e) {
            std::cout << "Exception: " << e.what() << std::endl;
        }

        ui->connectBtn->setText("Connect");
    } else {
        try{
            capnoTrainer.Connect(port1.toUtf8().constData(), port2.toUtf8().constData());
            // startBlockingFunction();
            ui->connectBtn->setText("Disconnect");
        } catch(const std::exception &e){
            std::cout << "Exception: " << e.what() << std::endl;
        }
        catch (...) {
            // Catch all handler
            std::cerr << "An exception occurred!" << std::endl;
        }

    }
}

//void MainWindow::startBlockingFunction() {

//    QThread* thread = new QThread;

//    CapnoWorker* worker = new CapnoWorker(&capnoTrainer);
//    worker->moveToThread(thread);

//    connect(thread, &QThread::started, worker, &CapnoWorker::doWork);
//    connect(worker, &CapnoWorker::finished, thread, &QThread::quit);
//    connect(worker, &CapnoWorker::finished, worker, &CapnoWorker::deleteLater);
//    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

//    thread->start();
//}
