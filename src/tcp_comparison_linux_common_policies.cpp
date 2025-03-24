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
    };
    
    // Load available congestion algorithms
    void load_available_algorithms() {
        std::ifstream alg_file("/proc/sys/net/ipv4/tcp_available_congestion_control");
        std::string alg;
        while (alg_file >> alg) {
            available_algorithms.push_back(alg);
        }
    }

    // Add this to your CongestionTester class
void save_results_to_csv(const std::string& filename, 
    const std::map<std::string, std::vector<Metrics>>& all_results) {
    std::ofstream csv_file(filename);

    std::vector<int> bandwidths = {10, 50, 100}; // Mbps
    std::vector<int> latencies = {5, 20, 100};   // ms
    // Write header
    csv_file << "Algorithm,Bandwidth,Latency,Throughput,PacketLoss,Jitter\n";

    // Write data
    for (const auto& [alg, results] : all_results) {
        for (size_t i = 0; i < results.size(); ++i) {
            const auto& metrics = results[i];

            // Calculate which bandwidth and latency config this was
            int bandwidth_index = i / latencies.size();
            int latency_index = i % latencies.size();

            csv_file << alg << ","
                     << bandwidths[bandwidth_index] << ","
                     << latencies[latency_index] << ","
                     << metrics.throughput << ","
                     << metrics.latency << ","
                     << metrics.packet_loss << ","
                     << metrics.jitter << "\n";
        }
    }
}

public:
    CongestionTester() {
        load_available_algorithms();
        
        // Default to cubic if available
        current_algorithm = "cubic";
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
    Metrics run_test(int duration_seconds, int bandwidth_limit_mbps) {
        // Create server socket
        // Setup client
        // Generate traffic according to parameters
        // Collect metrics
        
        Metrics result;
        // ... fill in metrics from test
        
        return result;
    }
    
    // Display available algorithms
    void show_available_algorithms() {
        std::cout << "Available congestion control algorithms:\n";
        for (const auto& alg : available_algorithms) {
            std::cout << "- " << alg << std::endl;
        }
    }
};

int main() {
    CongestionTester tester;
    tester.show_available_algorithms();
    
    // Test each algorithm
    std::vector<std::string> algorithms_to_test = {"cubic", "bbr", "reno"};
    
    for (const auto& alg : algorithms_to_test) {
        if (tester.set_congestion_algorithm(alg)) {
            std::cout << "Testing " << alg << "...\n";
            auto metrics = tester.run_test(30, 100);  // 30 seconds, 100Mbps limit
            // Report metrics
        }
    }
    
    return 0;
}