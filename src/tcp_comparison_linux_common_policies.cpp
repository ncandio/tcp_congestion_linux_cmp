#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>

using namespace std;

class CongestionTester {
private:
    std::string current_algorithm;
    std::vector<std::string> available_algorithms;
    
    // Socket file descriptors
    int server_fd;
    int client_fd;
    
    // Performance metrics
    struct Metrics {
        double throughput;        // In Mbps
        double latency;           // In ms
        int packet_loss;          // Count
        double jitter;            // In ms
        int bandwidth_config;     // Test configuration (Mbps)
        int latency_config;       // Test configuration (ms)
    };
    
    // Test results storage
    std::map<std::string, std::vector<Metrics>> all_results;
    
    // Load available congestion algorithms
    void load_available_algorithms() {
        std::ifstream alg_file("/proc/sys/net/ipv4/tcp_available_congestion_control");
        std::string alg;
        while (alg_file >> alg) {
            available_algorithms.push_back(alg);
        }
    }

    // Generate a CSV with detailed performance metrics
    void save_detailed_metrics(const std::string& filename) {
        std::ofstream csv_file(filename);
        
        // Write header
        csv_file << "Algorithm,BandwidthConfig,LatencyConfig,Throughput,Latency,PacketLoss,Jitter\n";
        
        // Write data
        for (const auto& [alg, results] : all_results) {
            for (const auto& metrics : results) {
                csv_file << alg << ","
                         << metrics.bandwidth_config << ","
                         << metrics.latency_config << ","
                         << metrics.throughput << ","
                         << metrics.latency << ","
                         << metrics.packet_loss << ","
                         << metrics.jitter << "\n";
            }
        }
        
        csv_file.close();
        std::cout << "Detailed metrics saved to " << filename << std::endl;
    }
    
    // Generate a CSV with algorithm comparison summary
    void save_algorithm_comparison(const std::string& filename) {
        std::ofstream csv_file(filename);
        
        // Calculate average metrics for each algorithm
        std::map<std::string, std::map<std::string, double>> avg_metrics;
        
        for (const auto& [alg, results] : all_results) {
            double avg_throughput = 0.0;
            double avg_latency = 0.0;
            double avg_packet_loss = 0.0;
            double avg_jitter = 0.0;
            int count = 0;
            
            for (const auto& metrics : results) {
                avg_throughput += metrics.throughput;
                avg_latency += metrics.latency;
                avg_packet_loss += metrics.packet_loss;
                avg_jitter += metrics.jitter;
                count++;
            }
            
            if (count > 0) {
                avg_metrics[alg]["throughput"] = avg_throughput / count;
                avg_metrics[alg]["latency"] = avg_latency / count;
                avg_metrics[alg]["packet_loss"] = avg_packet_loss / count;
                avg_metrics[alg]["jitter"] = avg_jitter / count;
            }
        }
        
        // Write header
        csv_file << "Algorithm,AvgThroughput,AvgLatency,AvgPacketLoss,AvgJitter\n";
        
        // Write data
        for (const auto& [alg, metrics] : avg_metrics) {
            csv_file << alg << ","
                     << metrics.at("throughput") << ","
                     << metrics.at("latency") << ","
                     << metrics.at("packet_loss") << ","
                     << metrics.at("jitter") << "\n";
        }
        
        csv_file.close();
        std::cout << "Algorithm comparison saved to " << filename << std::endl;
    }

public:
    CongestionTester() {
        load_available_algorithms();
        
        // Default to cubic if available
        current_algorithm = "cubic";
        
        // Initialize socket descriptors
        server_fd = -1;
        client_fd = -1;
    }
    
    // Destructor to clean up resources
    ~CongestionTester() {
        if (server_fd >= 0) {
            close(server_fd);
        }
        if (client_fd >= 0) {
            close(client_fd);
        }
    }
    
    // Set congestion algorithm for testing
    bool set_congestion_algorithm(const std::string& algorithm) {
        // Check if algorithm is available
        bool available = false;
        for (const auto& alg : available_algorithms) {
            if (alg == algorithm) {
                available = true;
                break;
            }
        }
        
        if (!available) {
            std::cerr << "Algorithm " << algorithm << " not available\n";
            return false;
        }
        
        // Set the algorithm system-wide (for testing purposes)
        std::ofstream cc_file("/proc/sys/net/ipv4/tcp_congestion_control");
        cc_file << algorithm;
        
        current_algorithm = algorithm;
        std::cout << "Set congestion control to: " << algorithm << std::endl;
        return true;
    }
    
    // Run a test with the current algorithm
    Metrics run_test(int duration_seconds, int bandwidth_limit_mbps, int latency_ms) {
        std::cout << "Running test with " << current_algorithm 
                  << " (Bandwidth: " << bandwidth_limit_mbps << " Mbps, Latency: " << latency_ms << " ms)\n";
        
        // Setup server if not already done
        if (server_fd < 0) {
            setup_server();
        }
        
        // Setup client connection
        setup_client();
        
        // Apply network conditions
        apply_network_conditions(bandwidth_limit_mbps, latency_ms);
        
        // Run the actual test
        Metrics result = measure_performance(duration_seconds);
        
        // Store configuration parameters
        result.bandwidth_config = bandwidth_limit_mbps;
        result.latency_config = latency_ms;
        
        // Store results
        all_results[current_algorithm].push_back(result);
        
        return result;
    }
    
    // Run a comprehensive test suite
    void run_test_suite() {
        std::vector<int> bandwidths = {10, 50, 100}; // Mbps
        std::vector<int> latencies = {5, 20, 100};   // ms
        
        for (const auto& alg : available_algorithms) {
            if (set_congestion_algorithm(alg)) {
                std::cout << "\n=== Testing " << alg << " ===\n";
                
                for (int bw : bandwidths) {
                    for (int lat : latencies) {
                        auto metrics = run_test(30, bw, lat);  // 30 seconds duration
                        
                        // Display results
                        std::cout << "Results: " 
                                  << "BW=" << bw << "Mbps, "
                                  << "Lat=" << lat << "ms -> "
                                  << "Throughput=" << metrics.throughput << "Mbps, "
                                  << "Latency=" << metrics.latency << "ms, "
                                  << "PacketLoss=" << metrics.packet_loss << ", "
                                  << "Jitter=" << metrics.jitter << "ms\n";
                    }
                }
            }
        }
    }
    
    // Save results to CSV files
    void save_results() {
        save_detailed_metrics("detailed_metrics.csv");
        save_algorithm_comparison("algorithm_comparison.csv");
    }
    
    // Display available algorithms
    void show_available_algorithms() {
        std::cout << "Available congestion control algorithms:\n";
        for (const auto& alg : available_algorithms) {
            std::cout << "- " << alg << std::endl;
        }
    }
    
private:
    // Setup server socket
    void setup_server() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "Failed to create server socket\n";
            return;
        }
        
        // Set socket options
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            std::cerr << "Failed to set socket options\n";
            close(server_fd);
            server_fd = -1;
            return;
        }
        
        // Bind to port
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(5000);  // Use port 5000 for testing
        
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            std::cerr << "Failed to bind socket\n";
            close(server_fd);
            server_fd = -1;
            return;
        }
        
        // Listen for connections
        if (listen(server_fd, 3) < 0) {
            std::cerr << "Failed to listen\n";
            close(server_fd);
            server_fd = -1;
            return;
        }
        
        std::cout << "Server initialized on port 5000\n";
    }
    
    // Setup client connection
    void setup_client() {
        client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd < 0) {
            std::cerr << "Failed to create client socket\n";
            return;
        }
        
        // Connect to server
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(5000);
        
        // Convert IPv4 address from text to binary
        if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address\n";
            close(client_fd);
            client_fd = -1;
            return;
        }
        
        if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Connection failed\n";
            close(client_fd);
            client_fd = -1;
            return;
        }
        
        std::cout << "Client connected to server\n";
    }
    
    // Apply network conditions (bandwidth and latency)
    void apply_network_conditions(int bandwidth_mbps, int latency_ms) {
        // This is a placeholder - in a real implementation, you would use
        // tc (traffic control) or similar tools to emulate network conditions
        
        std::cout << "Applied network conditions: " 
                  << bandwidth_mbps << " Mbps, " 
                  << latency_ms << " ms latency\n";
    }
    
    // Measure performance metrics
    Metrics measure_performance(int duration_seconds) {
        Metrics result;
        
        // This is a placeholder for actual measurement code
        // In a real implementation, you would transfer data and measure:
        // - Throughput by timing data transfer
        // - Latency using ping or similar
        // - Packet loss by comparing sent vs received packets
        // - Jitter by measuring variation in packet arrival times
        
        // For demonstration, we'll just use dummy values
        // In a real implementation, these would be calculated from actual measurements
        result.throughput = 85.7;  // e.g., 85.7 Mbps
        result.latency = 15.2;     // e.g., 15.2 ms
        result.packet_loss = 2;    // e.g., 2 packets lost
        result.jitter = 3.5;       // e.g., 3.5 ms jitter
        
        return result;
    }
};

int main() {
    CongestionTester tester;
    tester.show_available_algorithms();
    
    // Option 1: Test specific algorithms
    std::vector<std::string> algorithms_to_test = {"cubic", "bbr", "reno"};
    
    for (const auto& alg : algorithms_to_test) {
        if (tester.set_congestion_algorithm(alg)) {
            std::cout << "Testing " << alg << "...\n";
            // Run tests with different bandwidth and latency combinations
            tester.run_test(30, 10, 5);   // 30 sec, 10Mbps, 5ms
            tester.run_test(30, 50, 20);  // 30 sec, 50Mbps, 20ms
            tester.run_test(30, 100, 100); // 30 sec, 100Mbps, 100ms
        }
    }
    
    // Option 2: Run a comprehensive test suite
    // tester.run_test_suite();
    
    // Save results to CSV files
    tester.save_results();
    
    return 0;
}
