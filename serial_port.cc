#include <utility>

//
// Created by User on 2019/3/29.
//

#include "serial_port.hh"

#include <sstream>
#include <thread>

inline std::string error_info_string(std::string &&prefix, DWORD code, int line) noexcept {
	std::stringstream builder;
	builder << "error occurred in class serial_port, when "
	        << prefix << " with code " << code << std::endl
	        << __FILE__ << '(' << line << ')';
	return builder.str();
}

#define THROW(INFO, CODE, LINE) throw std::exception(error_info_string(INFO, CODE, LINE).c_str())
#define TRY(OPERATION, LINE) if(!OPERATION) THROW(#OPERATION, GetLastError(), LINE)

serial_port::serial_port(const serial_port_config &config,
                         received_t &&received)
		: received(std::move(received)) {
	
	auto temp = std::string(R"(\\.\)") + config.name;
	handle = CreateFileA(temp.c_str(),  // 串口名，`COM9` 之后需要前缀
	                     GENERIC_READ | GENERIC_WRITE, // 读和写
	                     0,                            // 独占模式
	                     nullptr,                      // 子进程无权限
	                     OPEN_EXISTING,                // 打开设备
	                     FILE_FLAG_OVERLAPPED,
	                     nullptr);
	
	if (handle == INVALID_HANDLE_VALUE) THROW("CreateFileA(...)", GetLastError(), __LINE__);
	
	// 设置端口设定
	DCB dcb;
	TRY(GetCommState(handle, &dcb), __LINE__);
	dcb.BaudRate = config.baud_rate;
	dcb.ByteSize = 8;
	TRY(SetCommState(handle, &dcb), __LINE__);
	
	// 设置超时时间
	COMMTIMEOUTS timeouts{3, 1, 0, 10, 0};
	TRY(SetCommTimeouts(handle, &timeouts), __LINE__);
	
	// 设置缓冲区容量
	TRY(SetupComm(handle, 2 * config.buffer_size, 14 * 32), __LINE__);
	
	// 订阅事件
	TRY(SetCommMask(handle, EV_RXCHAR), __LINE__);
	
	auto buffer_size = config.buffer_size;
	std::thread([buffer_size, this] {
		DWORD                event = 0;
		OVERLAPPED           overlapped{};
		std::vector<uint8_t> buffer(buffer_size);
		
		while (true) {
			do {
				overlapped.hEvent = CreateEventA(nullptr, true, false, nullptr);
				if (!WaitCommEvent(handle, &event, &overlapped)) {
					auto condition = GetLastError();
					if (condition != ERROR_IO_PENDING)
						THROW("wait event", condition, __LINE__);
				}
				
				DWORD progress = 0;
				GetOverlappedResult(handle, &overlapped, &progress, true);
				if (event == 0) return;
			} while (event != EV_RXCHAR);
			
			ReadFile(handle, buffer.data(), buffer.size(), nullptr, &overlapped);
			if (GetLastError() == ERROR_IO_PENDING) {
				DWORD actual = 0;
				GetOverlappedResult(handle, &overlapped, &actual, true);
				if (actual != 0)
					this->received(std::vector<uint8_t>(buffer.begin(), buffer.begin() + actual));
			}
		}
	}).detach();
}

serial_port::~serial_port() {
	SetCommMask(handle, 0);
	CloseHandle(handle);
}

size_t serial_port::send(const uint8_t *data, size_t size) {
	OVERLAPPED overlapped{};
	
	WriteFile(handle, data, size, nullptr, &overlapped);
	auto condition = GetLastError();
	if (condition == ERROR_IO_PENDING) {
		DWORD actual = 0;
		GetOverlappedResult(handle, &overlapped, &actual, true);
		return actual;
	} else
		THROW("send", condition, __LINE__);
}
