//
// Created by ydrml on 2019/3/31.
//

#include <thread>
#include <iostream>
#include "../main/serial_port.hh"

int main() {
	try {
        serial_port port("COM3", 115200);
		
		/** 此线程控制读线程 */
		std::thread([&port] {
			std::cout << "start <--" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
			port.break_read();
			std::cout << "break <--" << std::endl;
		}).detach();
		
		/** 启动主线程读 */
		uint8_t buffer[1];
		port.read(buffer, sizeof(buffer));
		
		std::this_thread::sleep_for(std::chrono::seconds(1));
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	
	return 0;
}
