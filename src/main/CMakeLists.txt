cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 11)

add_library(serial_port_async
        serial_port.hh
        serial_port.cc)
