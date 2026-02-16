#!/bin/bash
# Usage: ./new_pico_project.sh <project-name>
# Automatically creates a new Raspberry Pi Pico project with build scripts.

set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <project-name>"
    exit 1
fi

PROJECT_NAME="$1"
EXE_NAME=${2:-$PROJECT_NAME}  # default to project name if no executable name provided
BASE_DIR=$(dirname "$(realpath "$0")")
SDK_DIR="$BASE_DIR/pico-sdk"
PROJ_DIR="$BASE_DIR/$PROJECT_NAME"

# Check Pico SDK
if [ ! -d "$SDK_DIR" ]; then
    echo "ERROR: pico-sdk not found at $SDK_DIR"
    exit 1
fi

# Check if project already exists
if [ -d "$PROJ_DIR" ]; then
    echo "ERROR: Project '$PROJECT_NAME' already exists"
    exit 1
fi

echo "Creating Pico project: $PROJECT_NAME"
mkdir -p "$PROJ_DIR"

# -------------------------------
# Create main.c
# -------------------------------
cat > "$PROJ_DIR/main.c" <<'EOF'
#include "pico/stdlib.h"

#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif

// Initialize LED
int pico_led_init(void) {
#if defined(PICO_DEFAULT_LED_PIN)
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    return cyw43_arch_init();
#else
    return PICO_OK;
#endif
}

// Turn LED on/off
void pico_set_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}

int main() {
    int rc = pico_led_init();
    if(rc != PICO_OK) return 1;

    while (true) {
        pico_set_led(true);
        sleep_ms(LED_DELAY_MS);
        pico_set_led(false);
        sleep_ms(LED_DELAY_MS);
    }
}
EOF

# -------------------------------
# Create CMakeLists.txt
# -------------------------------
cat > "$PROJ_DIR/CMakeLists.txt" <<EOF
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project($PROJECT_NAME C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable($EXE_NAME
  main.c
)

target_include_directories($EXE_NAME PRIVATE
        \${CMAKE_CURRENT_LIST_DIR}
)

# Uncomment to enable USB stdio
# pico_enable_stdio_usb($EXE_NAME 1)

pico_add_extra_outputs($EXE_NAME)

target_link_libraries($EXE_NAME
  pico_stdlib
)
EOF


# -------------------------------
# Create build.sh
# -------------------------------
cat > "$PROJ_DIR/build.sh" <<'EOF'
#!/bin/bash
set -e

# Set Pico SDK path if not already set
export PICO_SDK_PATH=${PICO_SDK_PATH:-"$(dirname "$(realpath "$0")")/pico-sdk"}

mkdir -p build
cd build

cmake ..
# Use all CPU cores for make
if [[ "$OSTYPE" == "darwin"* ]]; then
    make -j$(sysctl -n hw.ncpu)
else
    make -j$(nproc)
fi
EOF

chmod +x "$PROJ_DIR/build.sh"

# -------------------------------
# Copy pico_sdk_import.cmake
# -------------------------------
cp ./pico-sdk/external/pico_sdk_import.cmake "$PROJ_DIR/pico_sdk_import.cmake"

# -------------------------------
# Create rebuild.sh
# -------------------------------
cat > "$PROJ_DIR/rebuild.sh" <<'EOF'
#!/bin/bash
set -e

cd build
# Use all CPU cores for make
if [[ "$OSTYPE" == "darwin"* ]]; then
    make -j$(sysctl -n hw.ncpu)
else
    make -j$(nproc)
fi
EOF

chmod +x "$PROJ_DIR/rebuild.sh"

# -------------------------------
# Done
# -------------------------------
echo "Project '$PROJECT_NAME' created successfully!"
echo ""
echo "To build:"
echo "  cd $PROJ_DIR"
echo "  ./build.sh"
echo ""
echo "To rebuild after changes:"
echo "  ./rebuild.sh"
