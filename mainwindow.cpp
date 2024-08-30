#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QSharedPointer>
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
                 false
          )
{
    ui->setupUi(this);


    // create objects for the label and progress bar
    statusLabel = new QLabel(this);
    petCO2Label = new QLabel(this);
    insCO2Label = new QLabel(this);
    bpmLabel = new QLabel(this);
    batteryLabel = new QLabel(this);
    heartRateLabel = new QLabel(this);
    spO2Label = new QLabel(this);



    // set text for the label
    heartRateLabel->setText("Heart Rate: 0 bpm\t");
    spO2Label->setText("SpO2: 0%\t");
    petCO2Label->setText("PetCO2: 0 (mmHg)\t");
    insCO2Label->setText("Insp. CO2: 0 (mmHg)\t");
    bpmLabel->setText("Resp. Rate: 0 (BPM)\t");
    batteryLabel->setText("Battery: N/A\t");
    statusLabel->setText("Status: N/A\t");

    // add the labels to the status bar
    ui->statusBar->addPermanentWidget(heartRateLabel);
    ui->statusBar->addPermanentWidget(spO2Label);
    ui->statusBar->addPermanentWidget(petCO2Label);
    ui->statusBar->addPermanentWidget(insCO2Label);
    ui->statusBar->addPermanentWidget(bpmLabel);
    ui->statusBar->addPermanentWidget(batteryLabel);
    ui->statusBar->addPermanentWidget(statusLabel);

    onRefreshPortBtnClicked();

    // Connect button signal to slot
    connect(ui->connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectBtnClicked);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshPortBtnClicked);
    connect(ui->clearGraphBtn, &QPushButton::clicked, this, &MainWindow::onClearGraphBtnClicked);

    // setup graph.
    ui->customPlot->addGraph(); // Add a graph
    ui->customPlot->addGraph(ui->customPlot->xAxis2, ui->customPlot->yAxis2); // Add a graph
    ui->customPlot->graph(0)->setPen(QPen(Qt::green));
    ui->customPlot->graph(1)->setPen(QPen(Qt::blue));


    ui->customPlot->xAxis->setLabel("Time (in sec.)");
    ui->customPlot->yAxis->setLabel("Raw PCO2 (in mmHg)");
    ui->customPlot->yAxis->setRange(0, 50);
    QSharedPointer<TimeAxisTicker> timeTicker(new TimeAxisTicker);
    ui->customPlot->xAxis->setTicker(timeTicker);

    ui->customPlot->xAxis2->setLabel("Time (in sec.)");
    ui->customPlot->yAxis2->setLabel("HRV (in msec.)");
    ui->customPlot->yAxis2->setRange(500, 1400);
    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->yAxis2->setVisible(true);
    QSharedPointer<TimeAxisTicker> timeTicker2(new TimeAxisTicker);
    ui->customPlot->xAxis2->setTicker(timeTicker2);


    // setup graph timer (10Hz is perceived as real time).
    connect(&graphPlotTimer, &QTimer::timeout, this, &MainWindow::updateGraph);
    graphPlotTimer.start(50);

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
    QVector<double> rrXData;
    QVector<double> rrYData;
    QMutexLocker locker(&dataMutex);

    while (!co2Queue.empty())
    {
        std::vector<float> data = co2Queue.front();
        co2Queue.pop();

        // here you can downsample the data.
        for (size_t i = 0; i < data.size(); i+=co2DataDownsample )
        {
            xData.push_back(((double)co2Samples / (co2Rate/co2DataDownsample)) );
            yData.push_back((double) data.at( i ));
            co2Samples += 1;
        }
    }

    while (!rrQueue.empty())
    {
        std::vector<float> data = rrQueue.front();
        rrQueue.pop();
        for (size_t i = 0; i < data.size(); i+=1 )
        {
            rrXData.push_back(( (double)rrSamples / rrRate ));
            rrYData.push_back((double) data.at( i ));
            rrSamples += 1;
        }
    }

    double max_time = 60*3.0 ; // in seconds

    // in case the queue is already empty (when the GO is not turned on).
    if (xData.size() > 0){
        ui->customPlot->graph(0)->addData(xData, yData);
        // make key axis range scroll with the data (at a constant range):
        ui->customPlot->xAxis->setRange( xData.at(xData.size()-1) +0.25, max_time, Qt::AlignRight);
        ui->customPlot->yAxis->setRange(0, 40);
        ui->customPlot->graph(0)->data()->removeBefore( xData.at(xData.size()-1) - max_time );
        ui->customPlot->replot();
    }


    if (rrXData.size() > 0){
        ui->customPlot->graph(1)->addData(rrXData, rrYData);
        // make key axis range scroll with the data (at a constant range):
        ui->customPlot->xAxis2->setRange( rrXData.at(rrXData.size()-1) +0.25, max_time, Qt::AlignRight);
        ui->customPlot->yAxis2->setRange(500 , 1200);
        ui->customPlot->graph(1)->data()->removeBefore( rrXData.at(rrXData.size()-1) - max_time );
        ui->customPlot->replot();
    }

}

// this function is called from libcapnotrainergo everytime
// a new data point is available on the USB dongle (either from GO, HRV, EMG or SpO2 devices)
void MainWindow::userCapnoCallback(std::vector<float> data, DeviceType device_type, uint8_t conn_handle, DataType data_type)
{
    QMutexLocker locker(&dataMutex);
    conn_handle;
    switch (device_type)
    {
        case DONGLE_DEVTYPE_CAPNO_GO:
        {
            if (data_type == DATA_CO2)
            {
                co2Queue.push(data);
            }
            if (data_type == DATA_BATTERY)
            {
                batteryLabel->setText(QString("Battery: %1%\t\t").arg(data.at(0)));
            }
            if (data_type == DATA_ETCO2_AVERAGE)
            {
                petCO2Label->setText(QString("PetCO2 (Avg.): %1 mmHg\t\t").arg(data.at(0)));
            }
            if (data_type == DATA_INSP_CO2_AVERAGE)
            {
                insCO2Label->setText(QString("Insp. CO2 (Avg.): %1 mmHg\t\t").arg(data.at(0)));
            }
            if (data_type == DATA_BPM_AVERAGE)
            {
                bpmLabel->setText(QString("Resp. Rate (Avg.): %1 BPM\t\t").arg(data.at(0)));
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
            }
        }
        break;

        case DONGLE_DEVTYPE_HRV:
        {
            if (data_type == DATA_RR_INTERVALS)
            {
                rrQueue.push(data);
            }
            if (data_type == DATA_HEART_RATE)
            {
                heartRateLabel->setText(QString("Heart Rate: %1 bpm\t").arg(data.at(0)));
            }
        }
        break;
        case DONGLE_DEVTYPE_O2_RING:
        {

            if (data_type == DATA_SPO2)
            {
                spO2Label->setText(QString("SpO2: %1%\t").arg(data.at(0)));
            }
            if (data_type == DATA_HEART_RATE)
            {
                heartRateLabel->setText(QString("Heart Rate: %1 bpm\t").arg(data.at(0)));
            }
            if (data_type == DATA_FINGER_PRESENT)
            {
            }
            if (data_type == DATA_MOTION_PRESENT)
            {
            }
            if (data_type == DATA_BATTERY)
            {
            }
            if (data_type == DATA_HR_STRENGTH)
            {
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

       // std::cout << ui->customPlot->graph(0)->dataCount() << std::endl;
       ui->customPlot->graph(0)->data()->clear();
       ui->customPlot->graph(1)->data()->clear();
       ui->customPlot->replot();
}

void MainWindow::onRefreshPortBtnClicked(){

    ui->comboBox1->clear();
    ui->comboBox2->clear();
    // Enumerate COM ports
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &info : availablePorts) {
        QString portName = info.portName();
        // probably add pid and vid filter here.
        ui->comboBox1->addItem(portName);
        ui->comboBox2->addItem(portName);
    }

    if ( availablePorts.size() >= 2 ){
        ui->comboBox1->setCurrentIndex(0);
        ui->comboBox1->setCurrentIndex(1);
    }
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
            //std::cout << "Exception: " << e.what() << std::endl;
            statusLabel->setText(QString("Status: %1\t").arg(e.what()));
        }

        ui->connectBtn->setText("Connect");
        statusLabel->setText("Status: Disconnected\t");
    } else {
        try{
            capnoTrainer.Connect(port1.toUtf8().constData(), port2.toUtf8().constData());
            ui->connectBtn->setText("Disconnect");
            statusLabel->setText("Status: Connected\t");
        } catch(const std::exception &e){
            std::cout << "Exception: " << e.what() << std::endl;
            statusLabel->setText(QString("Status: %1\t").arg(e.what()));
        }
        catch (...) {
            // std::cerr << "An exception occurred!" << std::endl;
            statusLabel->setText("Status: Unknown exception\t");
        }

    }
}
