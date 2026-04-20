cmake_minimum_required(VERSION 3.16)
project(V4L2Camera VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Build type default
# ---------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Detect whether we are cross-compiling
# The toolchain file sets CMAKE_CROSSCOMPILING automatically.
# ---------------------------------------------------------------------------
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for ${CMAKE_SYSTEM_PROCESSOR}")
else()
    message(STATUS "Native build on ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# ---------------------------------------------------------------------------
# Compiler warnings — applied to all targets
# ---------------------------------------------------------------------------
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
)

# ---------------------------------------------------------------------------
# V4L2CameraBase — control-plane only (no mmap / frame capture)
# Add new base source files here as the class grows.
# ---------------------------------------------------------------------------
add_library(V4L2CameraBase STATIC
    V4L2CameraBase.cpp
)

target_include_directories(V4L2CameraBase PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ---------------------------------------------------------------------------
# V4L2Camera — full pipeline (mmap buffers, streaming, frame capture)
# Depends on the base library so it inherits its include paths.
# Add new full-camera source files here.
# ---------------------------------------------------------------------------
add_library(V4L2Camera STATIC
    V4L2Camera.cpp
)

target_include_directories(V4L2Camera PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(V4L2Camera PUBLIC
    V4L2CameraBase
)

# ---------------------------------------------------------------------------
# Future camera components go here, e.g.:
#
#   add_library(MIPICamera STATIC MIPICamera.cpp)
#   target_link_libraries(MIPICamera PUBLIC V4L2Camera)
#
#   add_library(ParallelCamera STATIC ParallelCamera.cpp)
#   target_link_libraries(ParallelCamera PUBLIC V4L2Camera)
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# Testers
# ---------------------------------------------------------------------------

# Base tester — hardware control-plane test, no mmap
add_executable(v4l2base_tester
    v4l2base_tester.cpp
)
target_link_libraries(v4l2base_tester PRIVATE V4L2CameraBase)

# Full tester — exercises mmap, streaming, frame capture
add_executable(v4l2camera_tester
    v4l2camera_tester.cpp
)
target_link_libraries(v4l2camera_tester PRIVATE V4L2Camera)

# ---------------------------------------------------------------------------
# Install rules (optional — useful when integrating into F Prime later)
# ---------------------------------------------------------------------------
install(TARGETS V4L2CameraBase V4L2Camera
    ARCHIVE DESTINATION lib
)
install(FILES
    V4L2Device.hpp
    V4L2CameraBase.hpp
    V4L2Camera.hpp
    DESTINATION include/V4L2Camera
)