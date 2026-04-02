cmake_minimum_required(VERSION 3.16)
project(star-tracker-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(star_tracker_test star_tracker_test.cpp)
