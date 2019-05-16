cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 11)

add_library(serial_port_async
        macros.h
        serial_port.hh
        serial_port_win.cc
        serial_port_linux.cc)
