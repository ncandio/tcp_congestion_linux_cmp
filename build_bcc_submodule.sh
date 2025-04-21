#!/bin/bash
# Script to build BCC from the git submodule

set -e

# Check if BCC submodule exists
if [ ! -d "src/bcc" ]; then
    echo "BCC submodule not found. Adding it..."
    git submodule add -f https://github.com/iovisor/bcc.git src/bcc
    git submodule update --init --recursive
fi

# Build BCC
echo "Building BCC from submodule..."
mkdir -p src/bcc/build
cd src/bcc/build

# Configure with minimal components
cmake .. \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_LLVM_SHARED=ON \
    -DENABLE_MAN=OFF \
    -DENABLE_EXAMPLES=OFF \
    -DENABLE_TESTS=OFF \
    -DENABLE_TOOLS=OFF

# Build BCC library only
make -j$(nproc) bcc-shared

cd ../../..

echo "BCC built successfully!"
echo "You can now build your project with: mkdir -p build && cd build && cmake .. -DENABLE_EBPF_METRICS=ON && make"