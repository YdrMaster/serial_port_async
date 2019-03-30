#include <utility>

//
// Created by User on 2019/3/29.
//

#include "serial_port.hh"

#include <sstream>
#include <vector>

inline std::string error_info_string(std::string &&prefix, DWORD code, int line) noexcept {
	std::stringstream builder;
	builder << "error occurred in class serial_port, when "
	        << prefix << " with code " << code << std::endl
	        << __FILE__ << '(' << line << ')';
	return builder.str();
}

#define THROW(INFO, CODE, LINE) throw std::exception(error_info_string(INFO, CODE, LINE).c_str())
#define TRY(OPERATION, LINE) if(!OPERATION) THROW(#OPERATION, GetLastError(), LINE)

serial_port::serial_port(const std::string &name,
                         unsigned int baud_rate) {
	
	auto temp = std::string(R"(\\.\)") + name;
	handle = CreateFileA(temp.c_str(),  // 串口名，`COM9` 之后需要前缀
	                     GENERIC_READ | GENERIC_WRITE, // 读和写
	                     0,                            // 独占模式
	                     nullptr,                      // 子进程无权限
	                     OPEN_EXISTING,                // 打开设备
	                     FILE_FLAG_OVERLAPPED,
	                     nullptr);
	
	if (handle == INVALID_HANDLE_VALUE)
		THROW("CreateFileA(...)", GetLastError(), __LINE__);
	
	// 设置端口设定
	DCB dcb;
	TRY(GetCommState(handle, &dcb), __LINE__);
	dcb.BaudRate = baud_rate;
	dcb.ByteSize = 8;
	TRY(SetCommState(handle, &dcb), __LINE__);
	
	// 设置超时时间
	COMMTIMEOUTS timeouts{3, 1, 0, 10, 0};
	TRY(SetCommTimeouts(handle, &timeouts), __LINE__);
	
	// 设置缓冲区容量
	TRY(SetupComm(handle, 14 * 32, 14 * 32), __LINE__);
	
	// 订阅事件
	TRY(SetCommMask(handle, EV_RXCHAR), __LINE__);
}

serial_port::~serial_port() {
	SetCommMask(handle, 0);
	CloseHandle(handle);
}

void WINAPI callback(DWORD error_code,
                     DWORD actual,
                     LPOVERLAPPED overlapped) {
	if (error_code != ERROR_SUCCESS)
		THROW("WriteFileEx", error_code, __LINE__);
	
	delete static_cast<std::vector<uint8_t> *>(overlapped->hEvent);
	delete overlapped;
}

void serial_port::send(const uint8_t *buffer, size_t size) {
	auto overlapped = new OVERLAPPED{};
	auto ptr        = new std::vector<uint8_t>(buffer, buffer + size);
	overlapped->hEvent = ptr;
	WriteFileEx(handle, ptr->data(), size, overlapped, &callback);
	SleepEx(INFINITE, true);
}

size_t serial_port::read(uint8_t *buffer, size_t size) {
	DWORD      event = 0;
	OVERLAPPED overlapped{};
	
	do {
		overlapped.hEvent = CreateEventA(nullptr, true, false, nullptr);
		if (!WaitCommEvent(handle, &event, &overlapped)) {
			auto condition = GetLastError();
			if (condition != ERROR_IO_PENDING)
				THROW("WaitCommEvent", condition, __LINE__);
		}
		
		DWORD progress = 0;
		GetOverlappedResult(handle, &overlapped, &progress, true);
		if (event == 0) return 0;
	} while (event != EV_RXCHAR);
	
	ReadFile(handle, buffer, size, nullptr, &overlapped);
	auto condition = GetLastError();
	if (condition != ERROR_IO_PENDING)
		THROW("ReadFile", condition, __LINE__);
	DWORD actual = 0;
	GetOverlappedResult(handle, &overlapped, &actual, true);
	return actual;
}
