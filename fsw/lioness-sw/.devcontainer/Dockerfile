FROM ubuntu:22.04

# Prevent interactive prompts during package install
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=America/New_York

# Install everything needed
RUN apt-get update && apt-get install -y \
    # ARM cross-compiler
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    # Build tools
    cmake \
    ninja-build \
    make \
    # Python for F Prime tools
    python3 \
    python3-pip \
    # Git
    git \
    # SSH for deploying to EVK
    openssh-client \
    && rm -rf /var/lib/apt/lists/*

# Install F Prime tools
RUN pip3 install fprime-tools fprime-gds cmake

COPY lib/fprime/requirements.txt /tmp/fprime-requirements.txt
RUN pip3 install -r /tmp/fprime-requirements.txt

# Tell F Prime where the ARM toolchain is
ENV ARM_TOOLS_PATH=/usr/bin

WORKDIR /workspace
