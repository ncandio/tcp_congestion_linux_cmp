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
2. BCC tools installed:

```
sudo apt-get install bpfcc-tools python3-bpfcc
```

3. Python dependencies:
```
pip install pandas
```

## How It Works

1. When a TCP test starts, the C++ application launches the eBPF collector as a subprocess
2. The collector attaches to key TCP kernel functions:
   - `tcp_rcv_established`: Tracks established connections
   - `tcp_cwnd_event`: Captures congestion window changes
3. Metrics are collected and written to CSV files
4. The main application reads the CSV files to incorporate eBPF-collected metrics in its results

## Manual Usage

You can also run the eBPF collector independently:

```
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

For more advanced eBPF usage, refer to the [BCC documentation](https://github.com/iovisor/bcc).