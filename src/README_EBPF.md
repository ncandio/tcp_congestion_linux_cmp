# eBPF Integration for TCP Congestion Control Metrics

This document explains how the eBPF (Extended Berkeley Packet Filter) integration works in this project to collect detailed TCP congestion control metrics directly from the Linux kernel.

## Overview

eBPF allows us to attach probes to specific kernel functions and collect detailed metrics about TCP connections, including:

- Congestion window (cwnd) size
- Receive window (rwnd) size
- Round-trip time (RTT) and RTT variance
- Slow start threshold (ssthresh)
- Packet loss and retransmissions
- Throughput
- And more...

These metrics are collected in real-time during TCP congestion control tests.

## Files

- `tcp_ebpf_metrics.c`: The eBPF program that attaches to kernel TCP functions
- `tcp_ebpf_collector.py`: Python script that loads and runs the eBPF program and collects data
- Integration in `tcp_comparison_linux_common_policies.cpp` to start the collector during tests

## Prerequisites

To use the eBPF integration, you need:

1. Linux kernel 4.9+ (5.0+ recommended)
2. BCC (BPF Compiler Collection) - you have two options:

   **Option 1**: Install BCC from system packages:
   ```bash
   sudo apt-get install bpfcc-tools python3-bpfcc
   ```

   **Option 2**: Use the BCC git submodule (recommended for development):
   ```bash
   # Clone with the BCC submodule
   git clone --recurse-submodules https://your-repo-url.git
   
   # Or if already cloned, initialize the submodule
   git submodule update --init --recursive
   
   # Build the BCC submodule
   ./build_bcc_submodule.sh
   ```

3. Linux kernel headers for your running kernel:
   ```bash
   sudo apt-get install linux-headers-$(uname -r)
   ```

4. Python dependencies:
   ```bash
   pip install pandas
   ```

## Building with eBPF Support

To build the project with eBPF support:

```bash
# Create a build directory
mkdir -p build && cd build

# Configure with eBPF support
cmake .. -DENABLE_EBPF_METRICS=ON

# Build
make
```

The build system will automatically detect if you have BCC installed system-wide or as a git submodule.

## How It Works

1. When a TCP test starts, the C++ application launches the eBPF collector as a subprocess
2. The collector attaches to key TCP kernel functions:
   - `tcp_rcv_established`: Tracks established connections
   - `tcp_cwnd_event`: Captures congestion window changes
3. Metrics are collected and written to CSV files
4. The main application reads the CSV files to incorporate eBPF-collected metrics in its results

## Manual Usage

You can also run the eBPF collector independently:

```bash
python3 ./src/tcp_ebpf_collector.py --algorithm=cubic --duration=60 --output=my_metrics.csv
```

## Output Format

The collector generates CSV files with the following columns:

- timestamp: Event timestamp (ns)
- pid: Process ID
- comm: Process name
- src_ip/dst_ip: Source/destination IP addresses
- src_port/dst_port: Source/destination ports
- cwnd: Congestion window (packets)
- rwnd: Receive window (bytes)
- ssthresh: Slow start threshold
- rtt_us/rttvar_us: RTT and RTT variance (microseconds)
- bytes_sent/bytes_acked: Total bytes sent/acked
- lost_packets/retrans_packets: Packets lost/retransmitted
- cc_algo: Congestion control algorithm in use
- throughput_kbps: Calculated throughput

## Limitations

1. Requires root (sudo) privileges to run the eBPF program
2. Some metrics may not be available on all kernel versions
3. The collector adds some overhead to the system during measurements

## Troubleshooting

If you encounter issues:

1. Ensure you're running with sufficient privileges: `sudo ./tcp_test`
2. Check kernel headers are installed: `sudo apt-get install linux-headers-$(uname -r)`
3. Verify BCC is working: `sudo python3 -c "from bcc import BPF; print('BCC working')"`
4. If using the BCC submodule, make sure it was properly built: `./build_bcc_submodule.sh`
5. If CMake cannot find BCC, try specifying the path manually:
   ```bash
   cmake .. -DENABLE_EBPF_METRICS=ON -DBCC_INCLUDE_DIRS=/path/to/bcc/include -DBCC_LIBRARIES=/path/to/bcc/lib/libbcc.so
   ```
6. If you encounter missing libbcc.so errors when running:
   ```
   error while loading shared libraries: libbcc.so.0: cannot open shared object file: No such file or directory
   ```
   Try: `sudo ldconfig` or specify the library path: `LD_LIBRARY_PATH=/path/to/bcc/lib sudo ./tcp_test`

7. If BCC build fails with LLVM errors:
   ```bash
   sudo apt-get install llvm-dev libclang-dev
   ```

8. For Python import errors:
   ```bash
   # Check if bcc module is installed correctly
   sudo python3 -m pip install bcc
   ```
   
9. If the build system can't find the minimal headers, try:
   ```bash
   # Set environment variable to point to the headers
   export BCC_INCLUDE_DIR=/full/path/to/bcc_minimal/include
   ```
   
10. For "Failed to load BPF program" errors, check:
    - You have the correct kernel headers installed
    - Check syslog for details: `sudo dmesg | tail -30`
    - Verify program permission issues: `sudo cat /sys/kernel/debug/tracing/trace_pipe`

For more advanced eBPF usage, refer to the [BCC documentation](https://github.com/iovisor/bcc).