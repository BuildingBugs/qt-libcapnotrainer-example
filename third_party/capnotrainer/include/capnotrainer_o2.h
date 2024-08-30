#pragma once

#include "commons.h"


class CapnoTrainerO2
{

public:
	CapnoTrainerO2(uint8_t conn_handle, double current_time);

	uint8_t connection_handle;
	std::string device_name;
	double last_time;

	void handle_incoming_data(std::vector<uint8_t>& data, double current_time);
	std::vector<float> o2_array;
	std::vector<float> hr_array;
	std::vector<float> battery_array;
	std::vector<float> motion_array;
	std::vector<float> strength_array;
	std::vector<float> finger_array;
};