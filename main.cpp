#include <iostream>
#include <thread>
#include "serial_port.hh"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

int main() {
	int a = 0, b = 0;
	
	try {
		serial_port port({"COM3", 9600, 128},
		                 [&a](std::vector<uint8_t> &&data) {
			                 a += data.size();
			                 std::cout << a << std::endl;
		                 });
		
		const auto text = "abcde";
		while (true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			auto actual = port.send((uint8_t *) text, std::strlen(text));
			if (actual != 5) std::cerr << actual << std::endl;
			else {
				b += 5;
				std::cerr << b << std::endl;
			}
		}
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

#pragma clang diagnostic pop
