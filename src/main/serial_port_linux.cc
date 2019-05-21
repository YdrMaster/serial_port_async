//
// Created by User on 2019/3/29.
//

#include "serial_port.hh"

#ifdef __GNUC__

#include <vector>
#include <thread>
#include "macros.h"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>

#define TRY(OPERATION) if(!OPERATION) THROW(#OPERATION, std::strerror(errno))

inline int trans_baud(int number) {
    switch (number) {
        case 0:
            return B0;
        case 50:
            return B50;
        case 75:
            return B75;
        case 110:
            return B110;
        case 134:
            return B134;
        case 150:
            return B150;
        case 200:
            return B200;
        case 300:
            return B300;
        case 600:
            return B600;
        case 1200:
            return B1200;
        case 1800:
            return B1800;
        case 2400:
            return B2400;
        case 4800:
            return B4800;
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        case 460800:
            return B460800;
        case 500000:
            return B500000;
        case 576000:
            return B576000;
        case 921600:
            return B921600;
        case 1000000:
            return B1000000;
        case 1152000:
            return B1152000;
        case 1500000:
            return B1500000;
        case 2000000:
            return B2000000;
        case 2500000:
            return B2500000;
        case 3000000:
            return B3000000;
        case 3500000:
            return B3500000;
        case 4000000:
            return B4000000;
        default:
            THROW("set baud rate", "unsupported value");
    }
}

serial_port::serial_port(const std::string &name,
                         unsigned int baud_rate,
                         size_t in_buffer_size,
                         size_t out_buffer_size) {
    handle = open(name.c_str(), O_RDWR | O_NOCTTY);
    
    if (handle == -1)
        THROW("open(...)", std::strerror(errno));
    
    // 设置端口设定
    termios options{};
    TRY(!tcgetattr(handle, &options));
    cfsetispeed(&options, trans_baud(baud_rate));
    cfsetospeed(&options, trans_baud(baud_rate));
    options.c_cflag |= CS8;
    
    // 不超时
    options.c_cc[VTIME] = -1;
    
    // 原始模式
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    TRY(!tcsetattr(handle, TCSANOW, &options));
}

serial_port::~serial_port() {
    auto temp = handle.exchange(0);
    if (!temp) return;
    break_read();
    close(temp);
}

void serial_port::send(const uint8_t *buffer, size_t size) {
    if (size > 0) TRY(size == write(handle, buffer, size));
}

size_t serial_port::read(uint8_t *buffer, size_t size) {
    weak_lock_guard lock(read_mutex);
    return !lock ? 0 : ::read(handle, buffer, size);
}

void serial_port::break_read() const {
    weak_lock_guard lock(read_mutex);
}

#endif
