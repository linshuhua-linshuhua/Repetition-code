#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/json.hpp>
#include <boost/container/flat_map.hpp>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

struct RedfishUrlInfo {
    std::string sensorName;
    uint8_t propertyType;
    std::string thresholdName;
};

using PropertyMap = boost::container::flat_map<std::string, std::variant<
    std::vector<std::string>, std::string, int64_t, uint64_t,
    double, int32_t, uint32_t, int16_t, uint16_t, uint8_t, bool>>;

using AttributeTree = std::map<std::string, PropertyMap>;

class YinshanMonitor {
public:
    YinshanMonitor(net::io_context& ioc, const std::string& hostBmcIp, 
                   const std::string& username, const std::string& password);
    
    void propertiesRead();
    void startAsyncSensorFetch();
    
private:
    // Session management
    void createSession();
    void onSessionCreated(beast::error_code ec, http::response<http::string_body> res);
    
    // Sensor data fetching
    void fetchSensorData();
    void onSensorDataReceived(beast::error_code ec, http::response<http::string_body> res);
    
    // Data processing
    void processSensorData(const boost::json::object& sensorsJson);
    void matchAndAssignSensorValues(const boost::json::array& members);
    
    // SSL/HTTP helpers
    void performHttpsRequest(const std::string& target, http::verb method,
                           const std::string& body = "",
                           std::function<void(beast::error_code, http::response<http::string_body>)> callback = nullptr);
    
    // Member variables
    net::io_context& ioc_;
    ssl::context ctx_;
    std::unique_ptr<beast::ssl_stream<beast::tcp_stream>> stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    
    std::string hostBmcIp_;
    std::string username_;
    std::string password_;
    std::string authToken_;
    
    // Sensor mapping data
    const std::map<std::string, RedfishUrlInfo> yinshanDPUSensorCmdList = {
        {"DPU_NIC_OPT0_Temp", {"NIC_OPT0_Temp", PROPERTY_TEMP_SENSOR, "opt_temp"}},
        {"DPU_NIC_OPT0_Temp", {"NIC_OPT0_Temp", PROPERTY_TEMP_SENSOR, "opt_temp"}},
        {"DPU_NIC_OPT1_Temp", {"NIC_OPT1_Temp", PROPERTY_TEMP_SENSOR, "opt_temp"}},
        {"DPU_CPU_Temp", {"CPU_Temp", PROPERTY_TEMP_SENSOR, "cpu_temp"}},
        {"DPU_Cpu_Status", {"CPU_Status", PROPERTY_POWER_SENSOR, "status"}},
        {"DPU_Dimm0_Status", {"DIMM000", PROPERTY_POWER_SENSOR, "status"}},
        {"DPU_Dimm1_Status", {"DIMM010", PROPERTY_POWER_SENSOR, "status"}},
        {"DPU_M2_Status", {"M2_Status", PROPERTY_POWER_SENSOR, "status"}}
    };
    
    AttributeTree attributes_;
    std::map<std::string, bool> propertied_;
    
    // Constants
    static constexpr uint8_t PROPERTY_TEMP_SENSOR = 1;
    static constexpr uint8_t PROPERTY_POWER_SENSOR = 2;
    static constexpr int maxFrequency = 100;
    int frequency = 0;
    
    // Async operation state
    bool sessionActive_ = false;
    std::chrono::steady_clock::time_point lastFetchTime_;
};