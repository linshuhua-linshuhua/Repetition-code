#include "async_yinshan_monitor.hpp"
#include <boost/json.hpp>
#include <iostream>

AsyncYinshanMonitor::AsyncYinshanMonitor(net::io_context& ioc, const std::string& hostBmcIp, 
                                       const std::string& username, const std::string& password,
                                       std::chrono::seconds fetchInterval)
    : ioc_(ioc), ctx_(ssl::context::tlsv12_client), resolver_(ioc),
      hostBmcIp_(hostBmcIp), username_(username), password_(password),
      fetchInterval_(fetchInterval) {
    
    // Configure SSL context
    ctx_.set_default_verify_paths();
    ctx_.set_verify_mode(ssl::verify_none);
    
    // Initialize timer
    timer_ = std::make_unique<boost::asio::steady_timer>(ioc_);
    
    // Initialize attributes map
    for (const auto& [cmdKey, redfishInfo] : yinshanDPUSensorCmdList) {
        attributes_[cmdKey] = PropertyMap{};
    }
}

AsyncYinshanMonitor::~AsyncYinshanMonitor() {
    stop();
}

void AsyncYinshanMonitor::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    std::cout << "Starting async BMC sensor monitoring..." << std::endl;
    
    // Start with session creation
    asyncCreateSession();
}

void AsyncYinshanMonitor::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    sessionActive_ = false;
    
    if (timer_) {
        timer_->cancel();
    }
    
    if (stream_) {
        beast::error_code ec;
        stream_->shutdown(ec);
    }
    
    std::cout << "Async BMC sensor monitoring stopped." << std::endl;
}

void AsyncYinshanMonitor::scheduleNextFetch() {
    if (!running_) {
        return;
    }
    
    timer_->expires_after(fetchInterval_);
    timer_->async_wait([this](const boost::system::error_code& ec) {
        onTimerExpired(ec);
    });
}

void AsyncYinshanMonitor::onTimerExpired(const boost::system::error_code& ec) {
    if (ec || !running_) {
        return;
    }
    
    std::cout << "Timer expired, fetching sensor data..." << std::endl;
    
    if (!sessionActive_) {
        asyncCreateSession();
    } else {
        asyncFetchSensorData();
    }
}

void AsyncYinshanMonitor::asyncCreateSession() {
    std::cout << "Creating BMC session..." << std::endl;
    
    // Prepare session creation request
    boost::json::object sessionData;
    sessionData["UserName"] = username_;
    sessionData["Password"] = password_;
    
    std::string body = boost::json::serialize(sessionData);
    
    asyncPerformHttpsRequest("/redfish/v1/SessionService/Sessions", http::verb::post, body,
        [this](beast::error_code ec, http::response<http::string_body> res) {
            onAsyncSessionCreated(ec, std::move(res));
        });
}

void AsyncYinshanMonitor::onAsyncSessionCreated(beast::error_code ec, http::response<http::string_body> res) {
    if (ec) {
        std::cerr << "Async session creation failed: " << ec.message() << std::endl;
        scheduleNextFetch(); // Retry later
        return;
    }
    
    // Extract X-Auth-Token from response headers
    auto tokenIter = res.find("X-Auth-Token");
    if (tokenIter != res.end()) {
        authToken_ = std::string(tokenIter->value());
        sessionActive_ = true;
        std::cout << "Async session created successfully" << std::endl;
        
        // Immediately fetch sensor data
        asyncFetchSensorData();
    } else {
        std::cerr << "Failed to extract X-Auth-Token from async session response" << std::endl;
        scheduleNextFetch(); // Retry later
    }
}

void AsyncYinshanMonitor::asyncFetchSensorData() {
    if (!sessionActive_ || authToken_.empty()) {
        std::cerr << "No active session for async fetch" << std::endl;
        asyncCreateSession(); // Re-create session
        return;
    }
    
    std::cout << "Fetching sensor data asynchronously..." << std::endl;
    
    asyncPerformHttpsRequest("/redfish/v1/Chassis/1/Sensors", http::verb::get, "",
        [this](beast::error_code ec, http::response<http::string_body> res) {
            onAsyncSensorDataReceived(ec, std::move(res));
        });
}

void AsyncYinshanMonitor::onAsyncSensorDataReceived(beast::error_code ec, http::response<http::string_body> res) {
    if (ec) {
        std::cerr << "Async sensor data fetch failed: " << ec.message() << std::endl;
        // Session might have expired, try recreating
        sessionActive_ = false;
        scheduleNextFetch();
        return;
    }
    
    try {
        // Parse JSON response
        boost::json::value jsonValue = boost::json::parse(res.body());
        if (!jsonValue.is_object()) {
            std::cerr << "Invalid JSON response format in async fetch" << std::endl;
            scheduleNextFetch();
            return;
        }
        
        const auto& jsonObj = jsonValue.as_object();
        processSensorData(jsonObj);
        
        std::cout << "Async sensor data processing completed successfully" << std::endl;
        
        // Schedule next fetch
        scheduleNextFetch();
        
    } catch (const std::exception& e) {
        std::cerr << "JSON parsing error in async fetch: " << e.what() << std::endl;
        scheduleNextFetch();
    }
}

void AsyncYinshanMonitor::asyncPerformHttpsRequest(const std::string& target, http::verb method,
                                                  const std::string& body,
                                                  std::function<void(beast::error_code, http::response<http::string_body>)> callback) {
    
    // Store callback for later use
    currentCallback_ = callback;
    
    // Create new SSL stream for this request
    stream_ = std::make_unique<beast::ssl_stream<beast::tcp_stream>>(ioc_, ctx_);
    
    // Set SNI hostname
    if (!SSL_set_tlsext_host_name(stream_->native_handle(), hostBmcIp_.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        if (currentCallback_) {
            currentCallback_(ec, http::response<http::string_body>{});
        }
        return;
    }
    
    // Prepare HTTP request
    req_ = {};
    req_.version(11);
    req_.method(method);
    req_.target(target);
    req_.set(http::field::host, hostBmcIp_ + ":2443");
    req_.set(http::field::user_agent, "OpenBMC-AsyncYinshanMonitor/1.0");
    
    if (method == http::verb::post) {
        req_.set(http::field::content_type, "application/json");
        req_.body() = body;
        req_.prepare_payload();
    }
    
    // Add auth token if available
    if (!authToken_.empty()) {
        req_.set("X-Auth-Token", authToken_);
    }
    
    // Start async resolve
    resolver_.async_resolve(hostBmcIp_, "2443",
        [this](beast::error_code ec, tcp::resolver::results_type results) {
            if (ec) {
                if (currentCallback_) {
                    currentCallback_(ec, http::response<http::string_body>{});
                }
                return;
            }
            
            // Start async connect
            beast::get_lowest_layer(*stream_).async_connect(results,
                [this](beast::error_code ec, tcp::resolver::results_type::endpoint_type endpoint) {
                    onConnect(ec, endpoint);
                });
        });
}

void AsyncYinshanMonitor::onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
    if (ec) {
        if (currentCallback_) {
            currentCallback_(ec, http::response<http::string_body>{});
        }
        return;
    }
    
    // Start async SSL handshake
    stream_->async_handshake(ssl::stream_base::client,
        [this](beast::error_code ec) {
            onHandshake(ec);
        });
}

void AsyncYinshanMonitor::onHandshake(beast::error_code ec) {
    if (ec) {
        if (currentCallback_) {
            currentCallback_(ec, http::response<http::string_body>{});
        }
        return;
    }
    
    // Start async write
    http::async_write(*stream_, req_,
        [this](beast::error_code ec, std::size_t bytes_transferred) {
            onWrite(ec, bytes_transferred);
        });
}

void AsyncYinshanMonitor::onWrite(beast::error_code ec, std::size_t) {
    if (ec) {
        if (currentCallback_) {
            currentCallback_(ec, http::response<http::string_body>{});
        }
        return;
    }
    
    // Start async read
    buffer_.clear();
    http::async_read(*stream_, buffer_, res_,
        [this](beast::error_code ec, std::size_t bytes_transferred) {
            onRead(ec, bytes_transferred);
        });
}

void AsyncYinshanMonitor::onRead(beast::error_code ec, std::size_t) {
    // Call the callback with the result
    if (currentCallback_) {
        currentCallback_(ec, std::move(res_));
        currentCallback_ = nullptr;
    }
    
    // Gracefully close the stream
    stream_->async_shutdown([this](beast::error_code ec) {
        if (ec == net::error::eof) {
            ec = {};
        }
        if (ec) {
            std::cerr << "Async stream shutdown error: " << ec.message() << std::endl;
        }
    });
}

void AsyncYinshanMonitor::processSensorData(const boost::json::object& sensorsJson) {
    // Find Members array in the response
    auto membersIter = sensorsJson.find("Members");
    if (membersIter == sensorsJson.end() || !membersIter->value().is_array()) {
        std::cerr << "No Members array found in async sensor response" << std::endl;
        return;
    }
    
    const auto& members = membersIter->value().as_array();
    matchAndAssignSensorValues(members);
}

void AsyncYinshanMonitor::matchAndAssignSensorValues(const boost::json::array& members) {
    int matchedSensors = 0;
    
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
                auto& propertyMap = attributes_[cmdKey];
                
                // Assign reading value
                propertyMap["valueProperty"] = readingValue;
                propertyMap["lastUpdated"] = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                
                std::cout << "Async assigned sensor " << sensorName 
                          << " value: " << readingValue << " to " << cmdKey << std::endl;
                
                // Extract and store additional sensor information
                auto readingUnitsIter = sensorObj.find("ReadingUnits");
                if (readingUnitsIter != sensorObj.end() && readingUnitsIter->value().is_string()) {
                    propertyMap["units"] = std::string(readingUnitsIter->value().as_string().c_str());
                }
                
                auto readingTypeIter = sensorObj.find("ReadingType");
                if (readingTypeIter != sensorObj.end() && readingTypeIter->value().is_string()) {
                    propertyMap["readingType"] = std::string(readingTypeIter->value().as_string().c_str());
                }
                
                // Store threshold information
                auto thresholdsIter = sensorObj.find("Thresholds");
                if (thresholdsIter != sensorObj.end() && thresholdsIter->value().is_object()) {
                    const auto& thresholds = thresholdsIter->value().as_object();
                    
                    auto upperCritIter = thresholds.find("UpperCritical");
                    if (upperCritIter != thresholds.end() && upperCritIter->value().is_object()) {
                        const auto& upperCrit = upperCritIter->value().as_object();
                        auto threshReadingIter = upperCrit.find("Reading");
                        if (threshReadingIter != upperCrit.end() && !threshReadingIter->value().is_null()) {
                            if (threshReadingIter->value().is_double()) {
                                propertyMap["upperCriticalThreshold"] = threshReadingIter->value().as_double();
                            }
                        }
                    }
                }
                
                // Store status information
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
                
                matchedSensors++;
                break; // Found match, no need to continue searching
            }
        }
    }
    
    std::cout << "Async processing completed: matched " << matchedSensors << " sensors" << std::endl;
}