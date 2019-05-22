cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 11)

add_executable(serial_port_sample test_io.cpp)
target_link_libraries(serial_port_sample serial_port_async)
target_link_libraries(serial_port_sample pthread)
