
#include <iostream>
#include <thread>
#include <functional>
#include <chrono>

#include "mainwindow.h"
#include "commons.h"
#include "capnotrainer.h"

#include <QApplication>
#include <QSerialPortInfo>


void user_data_callback(std::vector<float> data, DeviceType device_type, uint8_t conn_handle, DataType data_type)
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


int start_qt(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

int main(int argc, char *argv[])
{

    // port names can be found based on
    // port enumeration and pid/vid values.
    const char* port1 = "COM15";
    const char* port2 = "COM16";

    std::cout << "CapnoTrainer: " << CapnoTrainer::GetVersion() << std::endl;

    try {
        CapnoTrainer capno(port1, port2, user_data_callback, true);
        std::thread t1(std::thread([&capno]() { capno.Initialize(); }));
        std::thread t2(std::thread([&argc, &argv] (){ start_qt(argc, argv); }));
        t1.join();
        t2.join();
    }
    catch (asio::system_error& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
