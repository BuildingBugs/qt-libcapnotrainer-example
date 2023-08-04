#pragma once

#include <iostream>
#include "commons.h"
#include "capnotrainer_go.h"
#include "capnotrainer_hrv.h"
#include "capnotrainer_emg.h"

#define CAPNOTRAINER_SERIAL_READ_SIZE 64
#define CAPNOTRAINER_SERIAL_WRITE_SIZE 16

typedef std::shared_ptr<asio::serial_port> serial_port_ptr;

typedef std::function<void(std::vector<float> data, DeviceType device_type, uint8_t conn_handle, DataType data_type)> user_cb_t;

class CapnoTrainer
{
public:
    CapnoTrainer(const char* port1, const char* port2, user_cb_t user_cb);
    ~CapnoTrainer();
    static const char* GetVersion() {
        return "v1.0.0";
    }

    void Initialize();
    void Open();
    void Write(std::vector<uint8_t>& buffer, uint8_t conn_handle);
    void Close();
    bool CheckGoDevice(std::vector<CapnoTrainerGo>::iterator& go_device);

protected:

    asio::io_context io;
    asio::serial_port serial_port_1;
    asio::serial_port serial_port_2;

private:

    user_cb_t user_cb;
    std::vector<std::thread> tg;

    uint8_t read_buffer[CAPNOTRAINER_SERIAL_READ_SIZE];

    std::vector<uint8_t> device_handles;
    std::vector<CapnoTrainerGo> go_devices;
    std::vector<CapnoTrainerHrv> hrv_devices;
    std::vector<CapnoTrainerEmg> emg_devices;

    void HandleParser(uint8_t *read_buffer);
    void HandleGoParser(uint8_t *read_buffer);
    void HandleHrvParser(std::vector<uint8_t>& data);
    void HandleEmgParser(std::vector<uint8_t>& data);
    void HandleNameParser(uint8_t *read_buffer);

    void HandleDeviceConnections(uint8_t conn_handle);

    

};
