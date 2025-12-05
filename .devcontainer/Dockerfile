# Use Ubuntu 24.04 as base image
FROM ubuntu:24.04

# Set environment variables to avoid interactive prompts and set the timezone
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Install all necessary packages in a single RUN command to reduce image layers
RUN apt-get update && apt-get install -y \
    software-properties-common \
    wget \
    curl \
    gnupg \
    build-essential \
    git \
    sudo \
    ninja-build \
    python3 \
    python3-pip \
    python3-full \
    pipx \
    lsb-release \
    # The following tools are often included in build-essential, but it is good to be explicit
    gdb \
    valgrind \
    cppcheck \
    doxygen \
    graphviz \
    ccache \
    pkg-config \
    make \
    # Code coverage tools
    lcov \
    && rm -rf /var/lib/apt/lists/*

# Install gcovr via pip with --break-system-packages (safe in containers)
RUN pip3 install --break-system-packages gcovr

# Install GCC-14
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    apt-get update && \
    apt-get install -y \
    gcc-14 \
    g++-14 \
    && rm -rf /var/lib/apt/lists/*

# Set GCC-14 as default (gcov-14 is included with gcc-14)
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100 && \
    update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-14 100

# Install CMake 3.30+ (latest stable version from Kitware)
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor - | \
    tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null && \
    apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" && \
    apt-get update && \
    apt-get install -y cmake && \
    rm -rf /var/lib/apt/lists/*

# Install LLVM/Clang 20 (latest available for Ubuntu 24.04)
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | \
    gpg --dearmor -o /etc/apt/trusted.gpg.d/llvm-snapshot.gpg && \
    add-apt-repository "deb http://apt.llvm.org/noble/ llvm-toolchain-noble-20 main" && \
    apt-get update && \
    apt-get install -y \
    clang-20 \
    clang++-20 \
    clang-tools-20 \
    clang-tidy-20 \
    clang-format-20 \
    clangd-20 \
    libc++-20-dev \
    libc++abi-20-dev \
    lld-20 \
    lldb-20 \
    && rm -rf /var/lib/apt/lists/*

# Set Clang-20 as alternatives
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-20 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-20 100 && \
    update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-20 100 && \
    update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-20 100 && \
    update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-20 100

# Set the working directory
WORKDIR /workspace

# Keep the container running
CMD ["/bin/bash"]
