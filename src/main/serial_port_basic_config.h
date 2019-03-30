//
// Created by ydrml on 2019/3/29.
//

#ifndef UNTITLED2_SERIAL_PORT_BASIC_CONFIG_H
#define UNTITLED2_SERIAL_PORT_BASIC_CONFIG_H

#include <string>
#include <Windows.h>

/** 串口配置 */
struct serial_port_basic_config {
	std::string  name;
	DCB          baud_rate;
	COMMTIMEOUTS timeouts;
	size_t       buffer_size_in,
	             buffer_size_out;
};

#endif //UNTITLED2_SERIAL_PORT_BASIC_CONFIG_H
