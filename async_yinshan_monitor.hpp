#pragma once

#include "yinshan_monitor.hpp"
#include <boost/asio/steady_timer.hpp>
#include <memory>

/**
 * Enhanced asynchronous version of YinshanMonitor with proper async/await pattern
 * This version uses Boost.Asio's async operations throughout for better performance
 */
class AsyncYinshanMonitor {
public:
    AsyncYinshanMonitor(net::io_context& ioc, const std::string& hostBmcIp, 
                       const std::string& username, const std::string& password,
                       std::chrono::seconds fetchInterval = std::chrono::seconds(30));
    
    ~AsyncYinshanMonitor();
    
    // Start the async monitoring process
    void start();
    
    // Stop the monitoring process
    void stop();
    
    // Get current sensor data
    const AttributeTree& getSensorData() const { return attributes_; }
    
private:
    void scheduleNextFetch();
    void onTimerExpired(const boost::system::error_code& ec);
    
    void asyncCreateSession();
    void onAsyncSessionCreated(beast::error_code ec, http::response<http::string_body> res);
    
    void asyncFetchSensorData();
    void onAsyncSensorDataReceived(beast::error_code ec, http::response<http::string_body> res);
    
    void asyncPerformHttpsRequest(const std::string& target, http::verb method,
                                 const std::string& body = "",
                                 std::function<void(beast::error_code, http::response<http::string_body>)> callback = nullptr);
    
    void onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type endpoint);
    void onHandshake(beast::error_code ec);
    void onWrite(beast::error_code ec, std::size_t bytes_transferred);
    void onRead(beast::error_code ec, std::size_t bytes_transferred);
    
    // Member variables
    net::io_context& ioc_;
    ssl::context ctx_;
    tcp::resolver resolver_;
    std::unique_ptr<beast::ssl_stream<beast::tcp_stream>> stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    
    std::string hostBmcIp_;
    std::string username_;
    std::string password_;
    std::string authToken_;
    
    // Timing
    std::unique_ptr<boost::asio::steady_timer> timer_;
    std::chrono::seconds fetchInterval_;
    
    // State management
    bool running_ = false;
    bool sessionActive_ = false;
    
    // Current operation callback
    std::function<void(beast::error_code, http::response<http::string_body>)> currentCallback_;
    
    // Sensor mapping and data (same as base class)
    const std::map<std::string, RedfishUrlInfo> yinshanDPUSensorCmdList = {
        {"DPU_NIC_OPT0_Temp", {"NIC_OPT0_Temp", 1, "opt_temp"}},
        {"DPU_NIC_OPT0_Temp", {"NIC_OPT0_Temp", 1, "opt_temp"}},
        {"DPU_NIC_OPT1_Temp", {"NIC_OPT1_Temp", 1, "opt_temp"}},
        {"DPU_CPU_Temp", {"CPU_Temp", 1, "cpu_temp"}},
        {"DPU_Cpu_Status", {"CPU_Status", 2, "status"}},
        {"DPU_Dimm0_Status", {"DIMM000", 2, "status"}},
        {"DPU_Dimm1_Status", {"DIMM010", 2, "status"}},
        {"DPU_M2_Status", {"M2_Status", 2, "status"}}
    };
    
    AttributeTree attributes_;
    
    // Helper methods
    void processSensorData(const boost::json::object& sensorsJson);
    void matchAndAssignSensorValues(const boost::json::array& members);
};