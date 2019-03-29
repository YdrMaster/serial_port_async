#include <iostream>
#include <thread>
#include "serial_port.hh"

int main() {
	serial_port port("COM4");
	
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		port << std::vector<uint8_t>{'a', 'b', 'c', 'd', 'e'};
	}
}
