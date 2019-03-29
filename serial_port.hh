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
	 * @param config   串口配置
	 * @param received 接收回调
	 */
	explicit serial_port(const serial_port_config &config,
	                     received_t &&received);
	
	~serial_port();
	
	serial_port &operator<<(std::vector<uint8_t> &&);

private:
	HANDLE           handle;
	std::mutex       lock;
	const received_t received;
};


#endif //UNTITLED2_SERIAL_PORT_HH
