#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <cmath>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    capnoTrainer(nullptr)
{
    ui->setupUi(this);

    // Enumerate COM ports
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &info : availablePorts) {
        QString portName = info.portName();
        ui->comboBox1->addItem(portName);
        ui->comboBox2->addItem(portName);
    }

    // Connect button signal to slot
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::onButtonClicked);

    // setup graph
    ui->customPlot->addGraph(); // Add a graph
    ui->customPlot->graph(0)->setPen(QPen(Qt::blue)); // Line color blue for first graph
    ui->customPlot->xAxis->setLabel("Time");
    ui->customPlot->yAxis->setLabel("Raw PCO2");
    ui->customPlot->yAxis->setRange(0, 55); // Assuming you want a range from 0 to 100

}

MainWindow::~MainWindow()
{
    delete capnoTrainer;
    delete ui;
}

void MainWindow::updateGraph(std::vector<float> data)
{
    std::cout << "I am here" << std::endl;
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double xSample = 0;
    for (auto &ySample : data)
    {
        xSample = (double) co2Samples / co2Rate;
        co2Samples += 1;
        ui->customPlot->graph(0)->addData(xSample, (double) ySample);
        std::cout << "Sample: " << xSample << "  " << ySample << std::endl;
    }


    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot->xAxis->setRange(xSample+0.25, xSample + 30, Qt::AlignRight);
    ui->customPlot->replot();
}

void MainWindow::userCapnoCallback(std::vector<float> data, DeviceType device_type, uint8_t conn_handle, DataType data_type)
{
    // this callback function is called every time a
    // new data point is received on the dongle.

    // the data point is parsed based on its device type and
    // passed to this function.

    // Users are instructed to deep copy paste before the
    // buffer goes out of scope.
    switch (device_type)
    {
    case DONGLE_DEVTYPE_CAPNO_GO:
    {
        if (data_type == DATA_CO2)
        {
            updateGraph(data);
            std::cout << "Received CO2 data with length: " << data.size() << "  with handle: " << (int)conn_handle << std::endl;
        }

        if (data_type == DATA_CAPNO_BATTERY)
        {
            std::cout << "Received Battery data with length: " << data.size() << "  with handle: " << (int)conn_handle << std::endl;
        }

        if (data_type == DATA_CAPNO_STATUS)
        {
            // capno status (future implementation)
        }
    }
    break;

    case DONGLE_DEVTYPE_EMG:
    {
        if (data_type == DATA_EMG)
        {
            std::cout << "Received EMG data with length: " << data.size() << "  with handle: " << (int)conn_handle << std::endl;
        }
    }
    break;

    case DONGLE_DEVTYPE_HRV:
    {
        if (data_type == DATA_RR_INTERVALS)
        {
            std::cout << "Receuved RR-interval data with length: " << data.size() << "  with handle: " << (int)conn_handle << std::endl;
        }
        if (data_type == DATA_HEART_RATE)
        {
            // Some HRV devices outputs heart rate
            // but we have not implemented it as it is
            // an average heart rate instead of an instanteneous one.
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

void MainWindow::onButtonClicked() {
    QString port1 = ui->comboBox1->currentText();
    QString port2 = ui->comboBox2->currentText();

    // Check if any of the ports is not selected
    if (port1.isEmpty() || port2.isEmpty()) {
        // Show a warning message
        QMessageBox::warning(this, "Warning", "Please select two COM ports before proceeding.");
        return;
    }

    if (ui->startButton->text() == "Disconnect")
    {
        capnoTrainer->Close();
        ui->startButton->setText("Connect");
    } else {

        auto userCallback = std::bind(&MainWindow::userCapnoCallback, this,
                                       std::placeholders::_1, std::placeholders::_2,
                                      std::placeholders::_3, std::placeholders::_4);

        capnoTrainer = new CapnoTrainer(port1.toUtf8().constData(), port2.toUtf8().constData(), userCallback, true);

        startBlockingFunction();

        ui->startButton->setText("Disconnect");
    }

}

void MainWindow::startBlockingFunction() {

    QThread* thread = new QThread;
    CapnoWorker* worker = new CapnoWorker(capnoTrainer);  // assuming capno is a member of MainWindow

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &CapnoWorker::doWork);
    connect(worker, &CapnoWorker::finished, thread, &QThread::quit);
    connect(worker, &CapnoWorker::finished, worker, &CapnoWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}
