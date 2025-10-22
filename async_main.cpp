#include "async_yinshan_monitor.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <signal.h>

// Global pointer for signal handling
std::unique_ptr<AsyncYinshanMonitor> g_monitor;
boost::asio::io_context* g_ioc = nullptr;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    if (g_monitor) {
        g_monitor->stop();
    }
    if (g_ioc) {
        g_ioc->stop();
    }
}

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        std::string hostBmcIp = "192.168.1.100"; // Default BMC IP
        std::string username = "admin";           // Default username
        std::string password = "admin";           // Default password
        
        if (argc >= 2) hostBmcIp = argv[1];
        if (argc >= 3) username = argv[2];
        if (argc >= 4) password = argv[3];
        
        std::cout << "BMC IP: " << hostBmcIp << std::endl;
        std::cout << "Username: " << username << std::endl;
        
        // Setup signal handlers
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        
        // IO context for async operations
        boost::asio::io_context ioc;
        g_ioc = &ioc;
        
        // Create AsyncYinshanMonitor instance with 30-second fetch interval
        g_monitor = std::make_unique<AsyncYinshanMonitor>(
            ioc, hostBmcIp, username, password, std::chrono::seconds(30));
        
        // Start the monitoring process
        std::cout << "Starting async BMC sensor monitoring..." << std::endl;
        g_monitor->start();
        
        // Run the IO context (this will block until stopped)
        std::cout << "Running IO context..." << std::endl;
        ioc.run();
        
        std::cout << "IO context finished." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}