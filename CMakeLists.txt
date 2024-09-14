cmake_minimum_required(VERSION 3.29)
project(DeadManSwitch)

set(CMAKE_CXX_STANDARD 20)

find_package(libevent REQUIRED)

add_executable(DeadManSwitch src/main.cpp)
target_link_libraries(DeadManSwitch PRIVATE libevent::core libevent::extra)