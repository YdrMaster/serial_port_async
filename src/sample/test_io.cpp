﻿#include <iostream>
#include <thread>
#include <cstring>
#include "../main/serial_port.hh"

int main() {
	try {
        serial_port port("/dev/ttyUSB0", 115200);
		
		std::thread([&port] {
			uint8_t buffer[128]{};
			while (true) {
				auto actual = port.read(buffer, sizeof(buffer) - 1);
				std::cout << std::string(buffer, buffer + actual) << std::endl;
			}
		}).detach();
		
		const auto text = "abcde";
        while (true) {
            port.send((uint8_t *) text, std::strlen(text));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}
