//
// Created by user on 5/19/19.
//

#include <iostream>
#include "../main/serial_port.hh"

int main() {
    try {
        serial_port("/dev/ttyUSB0");
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    
    return 0;
}
