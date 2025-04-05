# BCC (eBPF) Integration for TCP Congestion Metrics

This document explains the eBPF integration with the TCP Congestion Control Metrics project, and details the setup process.

## Overview

The TCP Congestion Control Metrics project uses eBPF to collect detailed TCP metrics directly from the Linux kernel. To do this, it relies on the BCC (BPF Compiler Collection) library, which can be integrated in several ways:

1. System-wide installation via package manager
2. Git submodule with full BCC source
3. Minimal header-only implementation (for development and testing)

## Installation Methods

### 1. System-wide BCC installation (recommended for production)

```bash
sudo apt-get install bpfcc-tools python3-bpfcc linux-headers-$(uname -r)
pip3 install pandas
```

### 2. BCC Git Submodule (for development)

```bash
# Clone the repository with submodules
git clone --recurse-submodules <repo-url>

# Or initialize after cloning
git submodule update --init --recursive

# Then build BCC dependencies (requires LLVM)
cd src && ./build_bcc_submodule.sh
```

### 3. Minimal Header-only Stubs (for testing and CI/CD)

The project includes header-only stubs that allow compilation without full BCC functionality. This is useful for:
- CI/CD pipelines where installing BCC would be difficult
- Development environments where eBPF functionality is not needed
- Systems where BCC installation is problematic

To use this method:

```bash
cd src && ./build_bcc_submodule.sh
```

This creates minimal header stubs in `src/bcc_minimal/include/bcc/`.

## Building the Project with eBPF Support

To build with eBPF metrics collection:

```bash
mkdir -p build && cd build
cmake .. -DENABLE_EBPF_METRICS=ON
make
```

## CMake Configuration

The CMake system will:

1. First check for system-wide BCC installation
2. Then check for the BCC submodule
3. Finally fall back to the minimal header stubs

You can see which mode is active in the CMake output:
- "eBPF metrics collection: ENABLED" - BCC support is available
- "eBPF support fully enabled - all dependencies found" - System BCC is used
- "Using BCC from submodule" - BCC submodule is used
- "Using minimal BCC headers" - Header stubs are used (limited functionality)

## Running with eBPF Support

When running with eBPF support enabled, you'll need:

1. Root privileges (sudo)
2. Linux kernel headers matching your running kernel
3. Python 3 with pandas library

Example:

```bash
sudo ./bin/tcp_comparison_linux
```

## Troubleshooting

1. If you see "eBPF metrics collection not available (BCC not found at build time)", check:
   - BCC installation: `sudo apt-get install bpfcc-tools python3-bpfcc`
   - Kernel headers: `sudo apt-get install linux-headers-$(uname -r)`
   - CMAKE flags: `-DENABLE_EBPF_METRICS=ON`

2. If you encounter runtime errors about BCC:
   - Run with sudo
   - Test BCC: `sudo python3 -c "from bcc import BPF; print('BCC works!')"`
   - Check Python dependencies: `pip3 install pandas`

3. For full BCC submodule build issues:
   - Install LLVM: `sudo apt-get install llvm clang`
   - Follow detailed instructions in BCC documentation

## Minimal Header Limitations

The minimal header stubs allow compilation but have these limitations:
- No actual eBPF program loading
- Data collection functions are stubbed out
- The application will fall back to synthetic data