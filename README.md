# Comparing TCP Congestion Control Algorithms in Linux (with eBPF)

## Motivation

This project aims to benchmark and compare different TCP congestion control algorithms available in Linux, including standard algorithms like cubic and reno, as well as newer ones like BBR and brutal. By collecting detailed performance metrics under various network conditions, we can analyze how each algorithm handles congestion, packet loss, and latency.

## Linux Version Used

Windows Subsystem for Linux 2 (WSL2) runs a real Linux kernel, which we've recompiled to include various 
TCP congestion control algorithms. These algorithms determine how TCP handles network congestion, which directly
impacts throughput, latency, and overall network performance. In particular, the version used is
6.6.75.1-microsoft-standard-WSL2+.

## Features

- Tests multiple TCP congestion control algorithms (cubic, reno, brutal, bbr, etc.)
- Collects detailed metrics for throughput, latency, packet loss, and jitter
- Uses eBPF for kernel-level metrics collection (when available)
- Configurable network conditions (bandwidth, latency)
- Outputs results to CSV files for analysis

## Installation

### Prerequisites

- Linux kernel 4.9+ (5.0+ recommended)
- C++17 compatible compiler
- CMake 3.10+
- **Required for eBPF metrics collection**:
  - BCC tools: `sudo apt-get install bpfcc-tools python3-bpfcc`
  - Python 3 with pandas: `pip3 install pandas`

> **Important**: To build and run the eBPF components, you must install BCC tools with: `sudo apt-get install bpfcc-tools python3-bpfcc`

### Building

```bash
mkdir build && cd build
cmake ..
make
```

The binary will be located in `build/bin/tcp_comparison_linux`.

## Usage

### Checking Current Algorithm

To check which algorithm is currently active:

```bash
sysctl net.ipv4.tcp_congestion_control
```

### Changing the Algorithm

To change to a different algorithm (for example, TCP Brutal):

```bash
sudo sysctl -w net.ipv4.tcp_congestion_control=brutal
```

You can replace brutal with any algorithm listed in the available algorithms output.

### Running Tests

After building, run the application:

```bash
sudo ./bin/tcp_comparison_linux
```

Root permissions are needed for changing the congestion control algorithm and for eBPF metrics collection.

## Sample Output

```
Available congestion control algorithms:
- reno
- cubic
- brutal
Set congestion control to: cubic
Testing cubic...
Running test with cubic (Bandwidth: 10 Mbps, Latency: 5 ms)
Server initialized on port 5000
Client connected to server
Starting eBPF collector for 30 seconds...
Applied network conditions: 10 Mbps, 5 ms latency
Running test with cubic (Bandwidth: 50 Mbps, Latency: 20 ms)
Client connected to server
Starting eBPF collector for 30 seconds...
Applied network conditions: 50 Mbps, 20 ms latency
Running test with cubic (Bandwidth: 100 Mbps, Latency: 100 ms)
Client connected to server
Starting eBPF collector for 30 seconds...
Applied network conditions: 100 Mbps, 100 ms latency
Algorithm bbr not available
Set congestion control to: reno
Testing reno...
Running test with reno (Bandwidth: 10 Mbps, Latency: 5 ms)
Client connected to server
Starting eBPF collector for 30 seconds...
Applied network conditions: 10 Mbps, 5 ms latency
Running test with reno (Bandwidth: 50 Mbps, Latency: 20 ms)
```

## Results

After running tests, two CSV files are generated:
- `detailed_metrics.csv`: Contains detailed metrics for each test case
- `algorithm_comparison.csv`: Contains summary statistics comparing algorithms

For more advanced eBPF metrics, see the eBPF-specific CSV files generated during tests.

## eBPF Integration

This project uses eBPF to collect detailed TCP metrics directly from the Linux kernel. When eBPF support is available, the system will collect more detailed and accurate metrics including:

- Real-time congestion window (cwnd) monitoring
- Packet loss detection
- Retransmission tracking
- Round-trip time (RTT) measurements
- And more...

For details on the eBPF integration, see [README_EBPF.md](src/README_EBPF.md).

## References

- [TCP Brutal](https://github.com/apernet/tcp-brutal) - Implementation of the Brutal congestion control algorithm
- [BCC Tools](https://github.com/iovisor/bcc) - Tools for BPF Compiler Collection
- [Linux TCP Implementation](https://github.com/torvalds/linux/tree/master/net/ipv4)

## Just for Fun

If you enjoy tech humor, check out this hilarious parody about networking protocols by Krazam:
- [If TCP/IP was a video call](https://www.youtube.com/watch?v=NAkAMDeo_NM)
