#include "yinshan_monitor.hpp"
#include <boost/json.hpp>
#include <iostream>
#include <sstream>

YinshanMonitor::YinshanMonitor(net::io_context& ioc, const std::string& hostBmcIp, 
                               const std::string& username, const std::string& password)
    : ioc_(ioc), ctx_(ssl::context::tlsv12_client), hostBmcIp_(hostBmcIp),
      username_(username), password_(password) {
    
    // Configure SSL context to accept self-signed certificates
    ctx_.set_default_verify_paths();
    ctx_.set_verify_mode(ssl::verify_none);
    
    // Initialize the SSL stream
    stream_ = std::make_unique<beast::ssl_stream<beast::tcp_stream>>(ioc_, ctx_);
    
    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream_->native_handle(), hostBmcIp_.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        std::cerr << "SSL SNI setup failed: " << ec.message() << std::endl;
    }
}

void YinshanMonitor::propertiesRead() {
    // Initialize attributes map for sensor data
    for (const auto& [cmd, regInfo] : yinshanDPUSensorCmdList) {
        auto cycleIter = attributes_.find(cmd);
        if (cycleIter == attributes_.end()) {
            continue;
        }
        
        const auto& freInfo = cycleIter->second;
        if (freInfo.find("priority") != freInfo.end()) {
            // Update frequency based on priority
            frequency = (maxFrequency == frequency) ? 0 : (frequency + 1);
        }
        
        propertied_[cmd] = true;
    }
    
    // Start async sensor data fetching
    startAsyncSensorFetch();
}

void YinshanMonitor::startAsyncSensorFetch() {
    if (!sessionActive_) {
        createSession();
    } else {
        fetchSensorData();
    }
}

void YinshanMonitor::createSession() {
    // Prepare session creation request
    boost::json::object sessionData;
    sessionData["UserName"] = username_;
    sessionData["Password"] = password_;
    
    std::string body = boost::json::serialize(sessionData);
    
    performHttpsRequest("/redfish/v1/SessionService/Sessions", http::verb::post, body,
        [this](beast::error_code ec, http::response<http::string_body> res) {
            onSessionCreated(ec, std::move(res));
        });
}

void YinshanMonitor::onSessionCreated(beast::error_code ec, http::response<http::string_body> res) {
    if (ec) {
        std::cerr << "Session creation failed: " << ec.message() << std::endl;
        return;
    }
    
    // Extract X-Auth-Token from response headers
    auto tokenIter = res.find("X-Auth-Token");
    if (tokenIter != res.end()) {
        authToken_ = std::string(tokenIter->value());
        sessionActive_ = true;
        std::cout << "Session created successfully, token: " << authToken_ << std::endl;
        
        // Now fetch sensor data
        fetchSensorData();
    } else {
        std::cerr << "Failed to extract X-Auth-Token from session response" << std::endl;
    }
}

void YinshanMonitor::fetchSensorData() {
    if (!sessionActive_ || authToken_.empty()) {
        std::cerr << "No active session or auth token available" << std::endl;
        return;
    }
    
    performHttpsRequest("/redfish/v1/Chassis/1/Sensors", http::verb::get, "",
        [this](beast::error_code ec, http::response<http::string_body> res) {
            onSensorDataReceived(ec, std::move(res));
        });
}

void YinshanMonitor::onSensorDataReceived(beast::error_code ec, http::response<http::string_body> res) {
    if (ec) {
        std::cerr << "Sensor data fetch failed: " << ec.message() << std::endl;
        return;
    }
    
    try {
        // Parse JSON response
        boost::json::value jsonValue = boost::json::parse(res.body());
        if (!jsonValue.is_object()) {
            std::cerr << "Invalid JSON response format" << std::endl;
            return;
        }
        
        const auto& jsonObj = jsonValue.as_object();
        processSensorData(jsonObj);
        
    } catch (const std::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}

void YinshanMonitor::processSensorData(const boost::json::object& sensorsJson) {
    // Find Members array in the response
    auto membersIter = sensorsJson.find("Members");
    if (membersIter == sensorsJson.end() || !membersIter->value().is_array()) {
        std::cerr << "No Members array found in sensor response" << std::endl;
        return;
    }
    
    const auto& members = membersIter->value().as_array();
    matchAndAssignSensorValues(members);
}

void YinshanMonitor::matchAndAssignSensorValues(const boost::json::array& members) {
    for (const auto& member : members) {
        if (!member.is_object()) {
            continue;
        }
        
        const auto& sensorObj = member.as_object();
        
        // Extract sensor name
        auto nameIter = sensorObj.find("Name");
        if (nameIter == sensorObj.end() || !nameIter->value().is_string()) {
            continue;
        }
        
        std::string sensorName = nameIter->value().as_string().c_str();
        
        // Extract reading value
        auto readingIter = sensorObj.find("Reading");
        if (readingIter == sensorObj.end()) {
            continue;
        }
        
        double readingValue = 0.0;
        if (readingIter->value().is_double()) {
            readingValue = readingIter->value().as_double();
        } else if (readingIter->value().is_int64()) {
            readingValue = static_cast<double>(readingIter->value().as_int64());
        } else {
            continue; // Skip if reading is not a number
        }
        
        // Match sensor name with RedfishUrlInfo mapping
        for (const auto& [cmdKey, redfishInfo] : yinshanDPUSensorCmdList) {
            if (redfishInfo.sensorName == sensorName) {
                // Found matching sensor, assign value to properties
                auto attrIter = attributes_.find(cmdKey);
                if (attrIter != attributes_.end()) {
                    auto& propertyMap = attrIter->second;
                    
                    // Assign reading value based on property type
                    if (redfishInfo.propertyType == PROPERTY_TEMP_SENSOR) {
                        propertyMap["valueProperty"] = readingValue;
                        std::cout << "Assigned temperature sensor " << sensorName 
                                  << " value: " << readingValue << " to " << cmdKey << std::endl;
                    } else if (redfishInfo.propertyType == PROPERTY_POWER_SENSOR) {
                        // For power/status sensors, might need different handling
                        propertyMap["valueProperty"] = readingValue;
                        std::cout << "Assigned power sensor " << sensorName 
                                  << " value: " << readingValue << " to " << cmdKey << std::endl;
                    }
                    
                    // Also extract and store threshold information if available
                    auto thresholdsIter = sensorObj.find("Thresholds");
                    if (thresholdsIter != sensorObj.end() && thresholdsIter->value().is_object()) {
                        const auto& thresholds = thresholdsIter->value().as_object();
                        
                        // Store critical thresholds
                        auto upperCritIter = thresholds.find("UpperCritical");
                        if (upperCritIter != thresholds.end() && upperCritIter->value().is_object()) {
                            const auto& upperCrit = upperCritIter->value().as_object();
                            auto readingIter = upperCrit.find("Reading");
                            if (readingIter != upperCrit.end() && !readingIter->value().is_null()) {
                                if (readingIter->value().is_double()) {
                                    propertyMap["upperCriticalThreshold"] = readingIter->value().as_double();
                                }
                            }
                        }
                    }
                    
                    // Extract status information
                    auto statusIter = sensorObj.find("Status");
                    if (statusIter != sensorObj.end() && statusIter->value().is_object()) {
                        const auto& status = statusIter->value().as_object();
                        auto healthIter = status.find("Health");
                        if (healthIter != status.end() && healthIter->value().is_string()) {
                            propertyMap["health"] = std::string(healthIter->value().as_string().c_str());
                        }
                        auto stateIter = status.find("State");
                        if (stateIter != status.end() && stateIter->value().is_string()) {
                            propertyMap["state"] = std::string(stateIter->value().as_string().c_str());
                        }
                    }
                }
                break; // Found match, no need to continue searching
            }
        }
    }
    
    // Update last fetch time
    lastFetchTime_ = std::chrono::steady_clock::now();
    std::cout << "Sensor data processing completed" << std::endl;
}

void YinshanMonitor::performHttpsRequest(const std::string& target, http::verb method,
                                        const std::string& body,
                                        std::function<void(beast::error_code, http::response<http::string_body>)> callback) {
    
    // Resolve hostname
    tcp::resolver resolver(ioc_);
    auto const results = resolver.resolve(hostBmcIp_, "2443");
    
    // Connect to the server
    beast::get_lowest_layer(*stream_).connect(results);
    
    // Perform SSL handshake
    stream_->handshake(ssl::stream_base::client);
    
    // Prepare HTTP request
    req_ = {};
    req_.version(11);
    req_.method(method);
    req_.target(target);
    req_.set(http::field::host, hostBmcIp_ + ":2443");
    req_.set(http::field::user_agent, "OpenBMC-YinshanMonitor/1.0");
    
    if (method == http::verb::post) {
        req_.set(http::field::content_type, "application/json");
        req_.body() = body;
        req_.prepare_payload();
    }
    
    // Add auth token if available
    if (!authToken_.empty()) {
        req_.set("X-Auth-Token", authToken_);
    }
    
    // Send HTTP request
    http::write(*stream_, req_);
    
    // Read HTTP response
    buffer_.clear();
    http::read(*stream_, buffer_, res_);
    
    // Call callback if provided
    if (callback) {
        callback(beast::error_code{}, std::move(res_));
    }
    
    // Gracefully close the stream
    beast::error_code ec;
    stream_->shutdown(ec);
    if (ec == net::error::eof) {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec = {};
    }
    if (ec) {
        std::cerr << "Stream shutdown error: " << ec.message() << std::endl;
    }
    
    // Recreate stream for next request
    stream_ = std::make_unique<beast::ssl_stream<beast::tcp_stream>>(ioc_, ctx_);
    if (!SSL_set_tlsext_host_name(stream_->native_handle(), hostBmcIp_.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        std::cerr << "SSL SNI setup failed: " << ec.message() << std::endl;
    }
}