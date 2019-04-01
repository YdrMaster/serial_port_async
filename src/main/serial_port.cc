#include <utility>

//
// Created by User on 2019/3/29.
//

#include "serial_port.hh"

#ifdef _MSC_VER

#include <sstream>
#include <vector>
#include <thread>
#include <Windows.h>
#include <iostream>

/** 受控锁 */
class weak_lock_guard {
	volatile bool locked;
	std::mutex    &lock;

public:
	explicit weak_lock_guard(std::mutex &lock)
			: lock(lock),
			  locked(lock.try_lock()) {}
	
	explicit operator bool() const { return locked; }
	
	bool retry() {
		return locked ? true : locked = lock.try_lock();
	}
	
	~weak_lock_guard() { if (locked) lock.unlock(); }
};

constexpr unsigned long EVENT = EV_RXCHAR | EV_TXEMPTY;

inline std::string error_info_string(std::string &&prefix, DWORD code, int line) noexcept;

#define THROW(INFO, CODE) throw std::exception(error_info_string(INFO, CODE, __LINE__).c_str())
#define TRY(OPERATION) if(!OPERATION) THROW(#OPERATION, GetLastError())

serial_port::serial_port(const std::string &name,
                         unsigned int baud_rate,
                         size_t in_buffer_size,
                         size_t out_buffer_size)
		: buffer_size(in_buffer_size) {
	
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
	TRY(SetCommMask(handle, EVENT));
}

serial_port::~serial_port() {
	auto temp = handle.exchange(nullptr);
	if (!temp) return;
	break_operation();
	CloseHandle(temp);
}

void WINAPI callback(DWORD error_code,
                     DWORD actual,
                     LPOVERLAPPED overlapped) {
	delete static_cast<std::vector<uint8_t> *>(overlapped->hEvent);
	delete overlapped;
	
	if (error_code != ERROR_SUCCESS)
		THROW("WriteFileEx", error_code);
}

void serial_port::write(const uint8_t *buffer, size_t size) {
	if (size <= 0) return;
	
	if (!write_flag.test_and_set()) {
		auto overlapped = new OVERLAPPED{};
		auto ptr        = new std::vector<uint8_t>(buffer, buffer + size);
		overlapped->hEvent = ptr;
		WriteFileEx(handle, ptr->data(), size, overlapped, &callback);
		SleepEx(INFINITE, true);
	} else {
		for (size_t i = 0; i < size; ++i)
			_o.push_back(buffer[i]);
	}
}

std::vector<uint8_t> serial_port::read() {
	std::vector<uint8_t> temp;
	while (!_i.empty()) {
		temp.push_back(_i.front());
		_i.pop_front();
	}
	return temp;
}

event_t serial_port::operator()() {
	weak_lock_guard lock(opration_mutex);
	if (!lock) return {};
	
	event_t    event{};
	OVERLAPPED overlapped{};
	
	do {
		overlapped.hEvent = CreateEventA(nullptr, true, false, nullptr);
		if (!WaitCommEvent(handle, (DWORD *) &event, &overlapped)) {
			auto condition = GetLastError();
			if (condition != ERROR_IO_PENDING)
				THROW("WaitCommEvent", condition);
		}
		
		DWORD progress = 0;
		GetOverlappedResult(handle, &overlapped, &progress, true);
	} while (event && !event.read() && !event.writen());
	
	if (event.writen()) {
		if (_o.empty())
			write_flag.clear();
		else {
			auto overlapped_ptr = new OVERLAPPED{};
			auto ptr            = new std::vector<uint8_t>;
			overlapped_ptr->hEvent = ptr;
			while (!_o.empty()) {
				ptr->push_back(_o.front());
				_o.pop_front();
			}
			WriteFileEx(handle, ptr->data(), ptr->size(), overlapped_ptr, &callback);
			SleepEx(INFINITE, true);
		}
	}
	
	if (event.read()) {
		std::vector<uint8_t> buffer(buffer_size);
		ReadFile(handle, buffer.data(), buffer_size, nullptr, &overlapped);
		auto condition = GetLastError();
		if (condition != ERROR_IO_PENDING)
			THROW("ReadFile", condition);
		DWORD actual = 0;
		GetOverlappedResult(handle, &overlapped, &actual, true);
		for (size_t i = 0; i < actual; ++i) _i.push_back(buffer[i]);
	}
	
	return event;
}

void serial_port::break_operation() const {
	weak_lock_guard lock(opration_mutex);
	
	while (!lock.retry()) {
		SetCommMask(handle, EVENT);
		std::this_thread::yield();
	}
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

#endif
