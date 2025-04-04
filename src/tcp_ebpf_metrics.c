#include <uapi/linux/ptrace.h>
#include <net/sock.h>
#include <bcc/proto.h>

// Define a struct to hold TCP congestion metrics
struct tcp_metrics_t {
    u32 pid;                   // Process ID
    u64 ts;                    // Timestamp
    char comm[TASK_COMM_LEN];  // Process name
    u32 saddr;                 // Source address
    u32 daddr;                 // Destination address
    u16 sport;                 // Source port
    u16 dport;                 // Destination port
    u64 cwnd;                  // Congestion window
    u64 rwnd;                  // Receive window
    u32 ssthresh;              // Slow start threshold
    u32 rtt;                   // Round trip time (μs)
    u32 rttvar;                // RTT variance
    u64 bytes_sent;            // Total bytes sent
    u64 bytes_acked;           // Total bytes acked
    u32 lost_packets;          // Count of packets lost
    u32 retrans_packets;       // Count of retransmitted packets
    char cc_algo[16];          // Congestion control algo name
};

BPF_PERF_OUTPUT(tcp_metrics_events);
BPF_HASH(flow_metrics, u32, struct tcp_metrics_t);

// Probe tcp_rcv_established to track received packets
int kprobe__tcp_rcv_established(struct pt_regs *ctx, struct sock *sk, struct sk_buff *skb)
{
    if (sk == NULL)
        return 0;
        
    // Get basic socket info
    u32 pid = bpf_get_current_pid_tgid() >> 32;
    struct tcp_metrics_t metrics = {};
    struct tcp_sock *tp = (struct tcp_sock *)sk;
    
    // Skip if not TCP or not IPv4
    if (sk->sk_family != AF_INET)
        return 0;
        
    // Fill metrics from sock structure
    metrics.pid = pid;
    metrics.ts = bpf_ktime_get_ns();
    bpf_get_current_comm(&metrics.comm, sizeof(metrics.comm));
    
    // Network information
    metrics.saddr = sk->sk_rcv_saddr;
    metrics.daddr = sk->sk_daddr;
    metrics.sport = sk->sk_num;
    metrics.dport = sk->sk_dport;
    
    // TCP congestion control metrics
    metrics.cwnd = tp->snd_cwnd;
    metrics.rwnd = tp->rcv_wnd;
    metrics.ssthresh = tp->snd_ssthresh;
    metrics.rtt = tp->srtt_us >> 3;  // srtt_us is stored <<3
    metrics.rttvar = tp->mdev_us >> 2;
    metrics.bytes_sent = tp->bytes_sent;
    metrics.bytes_acked = tp->bytes_acked;
    metrics.lost_packets = tp->lost_out;
    metrics.retrans_packets = tp->retrans_out;
    
    // Save congestion algorithm name
    bpf_probe_read_kernel_str(metrics.cc_algo, sizeof(metrics.cc_algo), 
                      tp->icsk_ca_ops->name);
    
    // Store metrics by socket address
    u32 key = pid;
    flow_metrics.update(&key, &metrics);
    
    // Emit event with metrics
    tcp_metrics_events.perf_submit(ctx, &metrics, sizeof(metrics));
    return 0;
}

// Probe tcp_cwnd_event to catch changes in congestion window
int kprobe__tcp_cwnd_event(struct pt_regs *ctx, struct sock *sk, enum tcp_cm_event event)
{
    if (sk == NULL)
        return 0;
        
    u32 pid = bpf_get_current_pid_tgid() >> 32;
    struct tcp_sock *tp = (struct tcp_sock *)sk;
    
    // Get metrics from flow_metrics and update
    struct tcp_metrics_t *metrics;
    u32 key = pid;
    metrics = flow_metrics.lookup(&key);
    if (metrics) {
        metrics->cwnd = tp->snd_cwnd;
        metrics->ssthresh = tp->snd_ssthresh;
        
        // Submit updated metrics
        tcp_metrics_events.perf_submit(ctx, metrics, sizeof(*metrics));
    }
    
    return 0;
}