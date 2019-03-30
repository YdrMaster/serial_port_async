//
// Created by User on 2019/3/29.
//

#ifndef UNTITLED2_SERIAL_PORT_HH
#define UNTITLED2_SERIAL_PORT_HH


#include "serial_port_basic_config.h"

/** 串口 */
class serial_port final {
public:
	/**
	 * 构造器
	 */
	explicit serial_port(const std::string &name,
	                     unsigned int baud_rate = 9600);
	
	/**
	 * 析构器
	 */
	~serial_port();
	
	/**
	 * 发送
	 * @return 实际送出的字节数
	 */
	void send(const uint8_t *, size_t);
	
	/**
	 * 读取
	 * @return 实际读取的字节数
	 */
	size_t read(uint8_t *, size_t);

private:
	HANDLE handle;
};


#endif //UNTITLED2_SERIAL_PORT_HH
