#include <utility>

//
// Created by User on 2019/3/29.
//

#include "serial_port.hh"

#include <sstream>
#include <thread>
#include <iostream>

inline std::string error_info_string(std::string &&prefix, int line) noexcept {
	std::stringstream builder;
	builder << "error occurred in class serial_port, when "
	        << prefix << " with code " << GetLastError() << std::endl
	        << __FILE__ << '(' << line << ')';
	return builder.str();
}

#define THROW(INFO, LINE) throw std::exception(error_info_string(INFO, LINE).c_str())
#define TRY(OPERATION, LINE) if(!OPERATION) THROW(#OPERATION, LINE)

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
	
	if (handle == INVALID_HANDLE_VALUE) THROW("CreateFileA(...)", __LINE__);
	
	// 设置端口设定
	DCB dcb;
	TRY(GetCommState(handle, &dcb), __LINE__);
	dcb.BaudRate = config.baud_rate;
	dcb.ByteSize = 8;
	TRY(SetCommState(handle, &dcb), __LINE__);
	
	// 设置超时时间
	COMMTIMEOUTS timeouts{1, 1, 0, 1, 0};
	TRY(SetCommTimeouts(handle, &timeouts), __LINE__);
	
	// 设置缓冲区容量
	TRY(SetupComm(handle, 14 * 32, 14 * 32), __LINE__);
	
	// 订阅事件
	TRY(SetCommMask(handle, EV_RXCHAR), __LINE__);
	
	auto buffer_size = config.buffer_size;
	std::thread([buffer_size, this] {
		DWORD                event;
		OVERLAPPED           overlapped{};
		std::vector<uint8_t> buffer(buffer_size);
		
		while (true) {
			do {
				overlapped.hEvent = CreateEventA(nullptr, true, false, nullptr);
				auto done = WaitCommEvent(handle, &event, &overlapped);
				if (!done && GetLastError() != ERROR_IO_PENDING)
					throw std::exception();
				
				DWORD progress;
				GetOverlappedResult(handle, &overlapped, &progress, true);
				if (event == 0) return;
			} while (event != EV_RXCHAR);
			
			std::lock_guard<std::mutex> _(lock);
			
			ReadFile(handle, buffer.data(), buffer.size(), nullptr, &overlapped);
			if (GetLastError() == ERROR_IO_PENDING) {
				DWORD actual = 0;
				GetOverlappedResult(handle, &overlapped, &actual, true);
				this->received(std::vector<uint8_t>(buffer.begin(), buffer.begin() + actual));
			}
		}
	}).detach();
}

serial_port::~serial_port() {
	SetCommMask(handle, 0);
	std::lock_guard<std::mutex> _(lock);
	CloseHandle(handle);
}

serial_port &serial_port::operator<<(std::vector<uint8_t> &&data) {
	OVERLAPPED overlapped{};
	
	WriteFile(handle, data.data(), data.size(), nullptr, &overlapped);
	if (GetLastError() == ERROR_IO_PENDING) {
		DWORD actual = 0;
		GetOverlappedResult(handle, &overlapped, &actual, true);
		if (actual != data.size()) THROW("WriteFile", __LINE__);
	}
	
	return *this;
}
