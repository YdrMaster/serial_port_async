#include <iostream>
#include <thread>
#include "serial_port.hh"

int main() {
	try {
		serial_port port({"COM3", 9600, 128},
		                 [](std::vector<uint8_t> &&data) {
			                 std::cout << "received: " << data.size() << std::endl;
			                 std::cout << std::string(data.begin(), data.end()) << std::endl;
		                 });
		
		while (true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			port << std::vector<uint8_t>{'a', 'b', 'c', 'd', 'e'};
		}
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}
