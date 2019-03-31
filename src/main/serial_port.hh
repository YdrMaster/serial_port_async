//
// Created by User on 2019/3/29.
//

#ifndef UNTITLED2_SERIAL_PORT_HH
#define UNTITLED2_SERIAL_PORT_HH


#include <string>
#include <atomic>
#include <mutex>

/** 串口 */
class serial_port final {
public:
	/**
	 * 构造器
	 */
	explicit serial_port(const std::string &name,
	                     unsigned int baud_rate = 9600,
	                     size_t in_buffer_size = 0xffff,
	                     size_t out_buffer_size = 0xffff);
	
	/**
	 * 析构器
	 */
	~serial_port();
	
	/**
	 * 发送
	 */
	void send(const uint8_t *, size_t);
	
	/**
	 * 读取
	 * @return 实际读取的字节数
	 */
	size_t read(uint8_t *, size_t);
	
	/**
	 * 中断正在阻塞的读操作
	 */
	void break_read() const;
	
	/**
	 * @return 仍未返回的读操作数量
	 */
	unsigned int read_count() const;

private:
	void               *handle;
	std::atomic_uint   read_counter;
	mutable std::mutex break_mutex;
};


#endif //UNTITLED2_SERIAL_PORT_HH
