#include <iostream>
#include <thread>
#include <cstring>
#include "../main/serial_port.hh"

int main() {
    try {
        serial_port port("/dev/ttyUSB0", 115200, 3, 1);
        
        std::thread([&port] {
            uint8_t buffer[128]{};
            while (true) {
                auto     actual = port.read(buffer, sizeof(buffer) - 1);
                for (int i      = 0; i < actual; ++i)
                    std::cout << (int) buffer[i] << " ";
                std::cout << std::endl;
            }
        }).detach();
        
        const uint8_t text[]{0x2d, 11, 10, 13};
        while (true) {
            port.send(text, sizeof(text));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
