cmake_minimum_required(VERSION 3.16)
project(libcamera-capture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED IMPORTED_TARGET libcamera)

add_executable(capture capture.cpp)
target_link_libraries(capture PRIVATE PkgConfig::LIBCAMERA)
