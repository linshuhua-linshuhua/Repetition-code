#include "yinshan_monitor.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        // IO context for async operations
        boost::asio::io_context ioc;
        
        // BMC connection parameters
        std::string hostBmcIp = "192.168.1.100"; // Replace with actual BMC IP
        std::string username = "admin";           // Replace with actual username
        std::string password = "admin";           // Replace with actual password
        
        // Create YinshanMonitor instance
        YinshanMonitor monitor(ioc, hostBmcIp, username, password);
        
        // Start the monitoring process
        std::cout << "Starting BMC sensor monitoring..." << std::endl;
        monitor.propertiesRead();
        
        // Run the IO context in a separate thread
        std::thread ioThread([&ioc]() {
            ioc.run();
        });
        
        // Main loop - fetch sensor data periodically
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(30)); // Fetch every 30 seconds
            
            std::cout << "Triggering sensor data fetch..." << std::endl;
            monitor.startAsyncSensorFetch();
        }
        
        // Join the IO thread (this won't be reached in the current implementation)
        ioThread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}