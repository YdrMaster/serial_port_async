#include <iostream>
#include <thread>
#include "serial_port.hh"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

int main() {
	try {
		serial_port port({"COM3", 9600, 128},
		                 [](std::vector<uint8_t> &&data) {
			                 std::cout << "received: " << data.size() << std::endl;
			                 std::cout << std::string(data.begin(), data.end()) << std::endl;
		                 });
		
		const auto text = "abcde";
		while (true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			auto actual = port.send((uint8_t *) text, std::strlen(text));
			if (actual != 5) std::cerr << actual << std::endl;
		}
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

#pragma clang diagnostic pop
