# Cpmpare TCP congestion policies in Linux (using eBPF)

## Motivation

## Linux version used
Windows Subsystem for Linux 2 (WSL2) runs a real Linux kernel, which we've recompiled to include various 
TCP congestion control algorithms. These algorithms determine how TCP handles network congestion, which directly
impacts throughput, latency, and overall network performance. In particular the version I'm used is a
6.6.75.1-microsoft-standard-WSL2+.


## Checking Current Algorithm
To check which algorithm is currently active:

```
sysctl net.ipv4.tcp_congestion_control
```

## Changing the Algorithm
To change to a different algorithm (for example, TCP Brutal):

```
sudo sysctl -w net.ipv4.tcp_congestion_control=brutal
```

You can replace brutal with any algorithm listed in the available algorithms output.

## Results

## How to start an emulated / simulated flow 

After cloning and building this repo, go in the bin folder and launch the application. Depending
on your configuration and Linux box , you will see the test run

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
Applied network conditions: 10 Mbps, 5 ms latency
Running test with cubic (Bandwidth: 50 Mbps, Latency: 20 ms)
Client connected to server
Applied network conditions: 50 Mbps, 20 ms latency
Running test with cubic (Bandwidth: 100 Mbps, Latency: 100 ms)
Client connected to server
Applied network conditions: 100 Mbps, 100 ms latency
Algorithm bbr not available
Set congestion control to: reno
Testing reno...
Running test with reno (Bandwidth: 10 Mbps, Latency: 5 ms)
Client connected to server
Applied network conditions: 10 Mbps, 5 ms latency
Running test with reno (Bandwidth: 50 Mbps, Latency: 20 ms)
```

## References

Hysteria / TCP brutal
https://github.com/apernet/tcp-brutal 
