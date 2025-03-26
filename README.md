# Cpmpare TCP congestion policies in Linux (using eBPF)

## Motivation

## Linux version used
Windows Subsystem for Linux 2 (WSL2) runs a real Linux kernel, which we've recompiled to include various 
TCP congestion control algorithms. These algorithms determine how TCP handles network congestion, which directly
impacts throughput, latency, and overall network performance. In particular the version I'm used is a
6.6.75.1-microsoft-standard-WSL2+.


## Checking Current Algorithm
To check which algorithm is currently active:

'''
sysctl net.ipv4.tcp_congestion_control
'''

## Changing the Algorithm
To change to a different algorithm (for example, TCP Brutal):

'''
sudo sysctl -w net.ipv4.tcp_congestion_control=brutal
'''
You can replace brutal with any algorithm listed in the available algorithms output.

## Results

## How to start an emulated / simulated flow 