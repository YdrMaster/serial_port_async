//
// Created by User on 2019/3/29.
//

#ifndef UNTITLED2_SERIAL_PORT_HH
#define UNTITLED2_SERIAL_PORT_HH


#include <string>
#include <vector>
#include <queue>
#include <atomic>
#include <mutex>
#include "event_t.h"

/** 串口 */
class serial_port final {
public:
	/**
	 * 构造器
	 */
	explicit serial_port(const std::string &name,
	                     unsigned int baud_rate = 9600,
	                     size_t in_buffer_size = 0x100,
	                     size_t out_buffer_size = 0x100);
	
	/**
	 * 析构器
	 */
	~serial_port();
	
	/**
	 * 发送
	 */
	void write(const uint8_t *buffer, size_t size);
	
	/**
	 * 读取
	 * @return 实际读取的字节数
	 */
	std::vector<uint8_t> read();
	
	/**
	 * 响应事件
	 *
	 * @return 此次响应的事件类型
	 */
	event_t operator()();
	
	/**
	 * 中断正在阻塞的读操作
	 */
	void break_operation() const;

private:
	std::atomic<void *> handle;
	mutable std::mutex  opration_mutex;
	
	
	const size_t     buffer_size;
	std::atomic_flag write_flag = ATOMIC_FLAG_INIT;
	
	std::mutex i_mutex, o_mutex;
	
	std::queue<uint8_t> i_, o_;
};


#endif //UNTITLED2_SERIAL_PORT_HH
