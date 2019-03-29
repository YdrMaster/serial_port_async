//
// Created by ydrml on 2019/3/29.
//

#ifndef UNTITLED2_SERIAL_PORT_CONFIG_H
#define UNTITLED2_SERIAL_PORT_CONFIG_H

#include <string>

/** 串口配置 */
struct serial_port_config {
	std::string  name;
	unsigned int baud_rate;
	size_t       buffer_size;
};

#endif //UNTITLED2_SERIAL_PORT_CONFIG_H
