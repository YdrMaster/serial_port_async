#include <iostream>
#include <thread>
#include "../main/serial_port.hh"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

int main() {
	try {
		serial_port port("COM3", 115200);
		
		std::thread([&port] {
			while (true) {
				auto event = port();
				if (!event) return;
				if (!event.read()) continue;
				auto buffer = port.read();
				std::cout << std::string(buffer.begin(), buffer.end()) << std::endl;
			}
		}).detach();
		
		const auto text = "abcdef";
		while (true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			port.write((uint8_t *) text, std::strlen(text));
		}
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

#pragma clang diagnostic pop
