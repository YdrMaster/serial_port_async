//
// Created by User on 2019/3/29.
//

#ifndef UNTITLED2_SERIAL_PORT_HH
#define UNTITLED2_SERIAL_PORT_HH


#include <vector>
#include <mutex>
#include <functional>
#include <Windows.h>
#include "serial_port_config.h"

/** 串口 */
class serial_port final {
public:
	/** 接收回调 */
	using received_t = std::function<void(std::vector<uint8_t> &&)>;
	
	/**
	 * 构造器
	 */
	explicit serial_port(const serial_port_config &, received_t &&);
	
	/**
	 * 析构器
	 */
	~serial_port();
	
	/**
	 * 发送
	 * @return 实际送出的字节数
	 */
	size_t send(const uint8_t *, size_t);

private:
	HANDLE           handle;
	std::mutex       lock;
	const received_t received;
};


#endif //UNTITLED2_SERIAL_PORT_HH
