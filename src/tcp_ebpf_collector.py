#!/usr/bin/env python3
from bcc import BPF
import time
import socket
import struct
import pandas as pd
import argparse
import os
import datetime

# Parse arguments
parser = argparse.ArgumentParser(description='TCP congestion control eBPF metrics collector')
parser.add_argument('--algorithm', type=str, help='TCP congestion algorithm to monitor')
parser.add_argument('--duration', type=int, default=60, help='Duration in seconds to collect data')
parser.add_argument('--output', type=str, default='ebpf_metrics.csv', help='Output CSV filename')
args = parser.parse_args()

# Get current congestion algorithm if not specified
if not args.algorithm:
    with open('/proc/sys/net/ipv4/tcp_congestion_control', 'r') as f:
        args.algorithm = f.read().strip()
    print(f"Monitoring TCP congestion algorithm: {args.algorithm}")

# Load eBPF program
b = BPF(src_file="tcp_ebpf_metrics.c")

# Process event
def print_event(cpu, data, size):
    event = b["tcp_metrics_events"].event(data)
    
    # Convert IP addresses to dotted-decimal notation
    saddr = socket.inet_ntoa(struct.pack("I", event.saddr))
    daddr = socket.inet_ntoa(struct.pack("I", event.daddr))
    
    # Convert ports from network to host byte order
    sport = socket.ntohs(event.sport)
    dport = socket.ntohs(event.dport)
    
    # Store event data in our collector
    metrics_data.append({
        'timestamp': event.ts,
        'pid': event.pid,
        'comm': event.comm.decode('utf-8'),
        'src_ip': saddr,
        'dst_ip': daddr,
        'src_port': sport,
        'dst_port': dport,
        'cwnd': event.cwnd,
        'rwnd': event.rwnd,
        'ssthresh': event.ssthresh,
        'rtt_us': event.rtt,
        'rttvar_us': event.rttvar,
        'bytes_sent': event.bytes_sent,
        'bytes_acked': event.bytes_acked,
        'lost_packets': event.lost_packets,
        'retrans_packets': event.retrans_packets,
        'cc_algo': event.cc_algo.decode('utf-8')
    })
    
    if len(metrics_data) % 100 == 0:
        print(f"Collected {len(metrics_data)} metrics records...")

# Create output directory if it doesn't exist
output_dir = os.path.dirname(args.output)
if output_dir and not os.path.exists(output_dir):
    os.makedirs(output_dir)

# Setup event handler
metrics_data = []
b["tcp_metrics_events"].open_perf_buffer(print_event)

print(f"Starting TCP metrics collection for {args.duration} seconds...")
start_time = time.time()

try:
    # Poll perf buffer
    while time.time() - start_time < args.duration:
        b.perf_buffer_poll(timeout=100)
except KeyboardInterrupt:
    print("Collection interrupted!")

# Save results to CSV
if metrics_data:
    # Convert to pandas DataFrame
    df = pd.DataFrame(metrics_data)
    
    # Convert timestamp to human-readable format
    df['time'] = pd.to_datetime(df['timestamp'], unit='ns')
    
    # Convert to human readable values and add units
    df['cwnd_packets'] = df['cwnd']
    df['rtt_ms'] = df['rtt_us'] / 1000
    df['rttvar_ms'] = df['rttvar_us'] / 1000
    df['rwnd_bytes'] = df['rwnd']
    
    # Calculate throughput and latency metrics
    time_diff = df['timestamp'].diff()
    df['throughput_kbps'] = df['bytes_acked'].diff() * 8 / time_diff * 1e6  # bytes to kbits/s
    
    # Add algorithm and collection metadata 
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    output_file = f"{args.algorithm}_{timestamp}_{args.output}"
    
    # Save to CSV
    df.to_csv(output_file, index=False)
    print(f"Metrics saved to {output_file}")
    
    # Generate summary statistics
    print("\nSummary Statistics:")
    print(f"Algorithm: {args.algorithm}")
    print(f"Total samples: {len(df)}")
    print(f"Average RTT: {df['rtt_ms'].mean():.2f} ms")
    print(f"Average CWND: {df['cwnd_packets'].mean():.2f} packets")
    print(f"Packet loss rate: {df['lost_packets'].sum() / df['bytes_sent'].sum() * 100:.4f}%")
    print(f"Average throughput: {df['throughput_kbps'].mean() / 1000:.2f} Mbps")
else:
    print("No data collected!")