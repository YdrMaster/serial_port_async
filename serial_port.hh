//
// Created by User on 2019/3/29.
//

#ifndef UNTITLED2_SERIAL_PORT_HH
#define UNTITLED2_SERIAL_PORT_HH


#include <string>
#include <vector>
#include <mutex>
#include <Windows.h>

class serial_port {
public:
	explicit serial_port(const std::string &port_name,
	                     unsigned int baud_rate = 9600,
	                     size_t buffer_size = 0xffff);
	
	~serial_port();
	
	serial_port &operator<<(std::vector<uint8_t> &&);

private:
	HANDLE     handle;
	std::mutex lock;
};


#endif //UNTITLED2_SERIAL_PORT_HH
