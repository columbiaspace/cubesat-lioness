cmake_minimum_required(VERSION 3.16)
project(V4L2Camera)

set(CMAKE_CXX_STANDARD 17)

# If cross-compiling, set toolchain via:
#   cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-imx.cmake ..
add_library(V4L2Camera STATIC
    V4L2Camera.cpp
)

target_include_directories(V4L2Camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Tester binary
add_executable(v4l2camera_tester
    v4l2camera_tester.cpp
)
target_link_libraries(v4l2camera_tester PRIVATE V4L2Camera)