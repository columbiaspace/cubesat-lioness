# toolchain-imx.cmake
# Pass to CMake with: cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-imx.cmake ..

set(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# ---------------------------------------------------------------------------
# Compiler — adjust prefix if your SDK uses a different one
# Common alternatives:
#   aarch64-poky-linux-
#   aarch64-fslc-linux-
# ---------------------------------------------------------------------------
set(CROSS_COMPILE aarch64-linux-gnu-)
set(CMAKE_C_COMPILER   ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)

# ---------------------------------------------------------------------------
# Sysroot — point at your Yocto / NXP SDK sysroot
# ---------------------------------------------------------------------------
set(CMAKE_SYSROOT /opt/fsl-imx-xwayland/6.1-langdale/sysroots/armv8a-poky-linux)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)