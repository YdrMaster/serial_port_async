#include <utility>

//
// Created by User on 2019/3/29.
//

#include "serial_port.hh"

#include <sstream>
#include <vector>
#include <thread>
#include <Windows.h>

/** 内存安全计数器 */
struct counter_guard {
	explicit counter_guard(std::atomic_uint &data)
			: data(data) { ++data; }
	
	counter_guard(const counter_guard &) = delete;
	
	counter_guard(counter_guard &&) = delete;
	
	~counter_guard() { --data; }

private:
	std::atomic_uint &data;
};

inline std::string error_info_string(std::string &&prefix, DWORD code, int line) noexcept;

#define THROW(INFO, CODE) throw std::exception(error_info_string(INFO, CODE, __LINE__).c_str())
#define TRY(OPERATION) if(!OPERATION) THROW(#OPERATION, GetLastError())

serial_port::serial_port(const std::string &name,
                         unsigned int baud_rate,
                         size_t in_buffer_size,
                         size_t out_buffer_size)
		: read_counter(0) {
	
	auto temp = std::string(R"(\\.\)") + name;
	handle = CreateFileA(temp.c_str(),  // 串口名，`COM9` 之后需要前缀
	                     GENERIC_READ | GENERIC_WRITE, // 读和写
	                     0,                            // 独占模式
	                     nullptr,                      // 子进程无权限
	                     OPEN_EXISTING,                // 打开设备
	                     FILE_FLAG_OVERLAPPED,
	                     nullptr);
	
	if (handle == INVALID_HANDLE_VALUE)
		THROW("CreateFileA(...)", GetLastError());
	
	// 设置端口设定
	DCB dcb;
	TRY(GetCommState(handle, &dcb));
	dcb.BaudRate = baud_rate;
	dcb.ByteSize = 8;
	TRY(SetCommState(handle, &dcb));
	
	// 设置超时时间
	COMMTIMEOUTS timeouts{3, 1, 0, 10, 0};
	TRY(SetCommTimeouts(handle, &timeouts));
	
	// 设置缓冲区容量
	TRY(SetupComm(handle, in_buffer_size, out_buffer_size));
	
	// 订阅事件
	TRY(SetCommMask(handle, EV_RXCHAR));
}

serial_port::~serial_port() {
	break_read();
	CloseHandle(handle);
}

void WINAPI callback(DWORD error_code,
                     DWORD actual,
                     LPOVERLAPPED overlapped) {
	delete static_cast<std::vector<uint8_t> *>(overlapped->hEvent);
	delete overlapped;
	
	if (error_code != ERROR_SUCCESS)
		THROW("WriteFileEx", error_code);
}

void serial_port::send(const uint8_t *buffer, size_t size) {
	if (size <= 0) return;
	
	auto overlapped = new OVERLAPPED{};
	auto ptr        = new std::vector<uint8_t>(buffer, buffer + size);
	overlapped->hEvent = ptr;
	WriteFileEx(handle, ptr->data(), size, overlapped, &callback);
	SleepEx(INFINITE, true);
}

size_t serial_port::read(uint8_t *buffer, size_t size) {
	counter_guard _(read_counter);
	
	DWORD      event = 0;
	OVERLAPPED overlapped{};
	
	do {
		overlapped.hEvent = CreateEventA(nullptr, true, false, nullptr);
		if (!WaitCommEvent(handle, &event, &overlapped)) {
			auto condition = GetLastError();
			if (condition != ERROR_IO_PENDING)
				THROW("WaitCommEvent", condition);
		}
		
		DWORD progress = 0;
		GetOverlappedResult(handle, &overlapped, &progress, true);
		if (event == 0) return 0;
	} while (event != EV_RXCHAR);
	
	ReadFile(handle, buffer, size, nullptr, &overlapped);
	auto condition = GetLastError();
	if (condition != ERROR_IO_PENDING)
		THROW("ReadFile", condition);
	DWORD actual = 0;
	GetOverlappedResult(handle, &overlapped, &actual, true);
	return actual;
}

void serial_port::break_read() const {
	std::lock_guard<std::mutex> _(break_mutex);
	while (read_counter > 0) {
		SetCommMask(handle, EV_RXCHAR);
		std::this_thread::yield();
	}
}

uint32_t serial_port::read_count() const {
	return read_counter.load();
}

std::string error_info_string(std::string &&prefix,
                              DWORD code,
                              int line) noexcept {
	std::stringstream builder;
	builder << "error occurred in class serial_port, when "
	        << prefix << " with code " << code << std::endl
	        << __FILE__ << '(' << line << ')';
	return builder.str();
}
