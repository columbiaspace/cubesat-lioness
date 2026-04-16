#!/bin/bash

if ! docker image inspect cubesat-dev &>/dev/null; then
    echo "Image not found, building..."
    docker build -t cubesat-dev -f .devcontainer/Dockerfile .
fi

echo "Available build targets:"
echo "  fprime-util generate imx8x   # Variscite EVK / Flight Computer"
echo "  fprime-util generate rpi5    # Raspberry Pi 5"
echo "  fprime-util generate native  # Local machine (for unit tests)"
echo ""

docker run -it --rm \
  -v $(pwd):/workspace \
  -v ~/.ssh:/root/.ssh:ro \
  cubesat-dev bash 
