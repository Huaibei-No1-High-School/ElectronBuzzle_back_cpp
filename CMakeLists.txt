cmake_minimum_required(VERSION 3.25)
project(electronBuzzer)

set(CMAKE_CXX_STANDARD 23)

find_package(libhv CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_executable(electronBuzzer main.cc config.h Connection.hpp)

target_link_libraries(electronBuzzer PRIVATE hv_static glog::glog yaml-cpp)