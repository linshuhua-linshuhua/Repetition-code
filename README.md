# OpenBMC Yinshan Monitor

This project implements an asynchronous BMC sensor data fetcher for OpenBMC systems, specifically designed to work with Redfish APIs. It fetches sensor data from BMC endpoints and maps them to predefined sensor configurations.

## Features

- **Asynchronous Operation**: Uses Boost.Asio for non-blocking I/O operations
- **Session Management**: Automatically creates and manages Redfish sessions with X-Auth-Token authentication
- **SSL/TLS Support**: Secure HTTPS communication with BMC endpoints
- **Sensor Mapping**: Maps BMC sensor names to internal sensor configurations
- **Periodic Fetching**: Configurable interval-based sensor data collection
- **Error Handling**: Robust error handling with automatic session recreation
- **Threshold Support**: Extracts and stores sensor threshold information

## API Endpoints Used

### Session Creation
- **URL**: `https://HOSTBMCIP:2443/redfish/v1/SessionService/Sessions`
- **Method**: POST
- **Data**: `{"UserName": "username", "Password": "password"}`
- **Headers**: `Content-Type: application/json`
- **Response**: X-Auth-Token in response headers

### Sensor Data Retrieval
- **URL**: `https://HOSTBMCIP:2443/redfish/v1/Chassis/1/Sensors`
- **Method**: GET
- **Headers**: `X-Auth-Token: <token_from_session>`

## Sensor Data Format

The API returns sensor data in the following JSON format:

```json
{
  "@odata.type": "#Sensor.v1_4_0.Sensor",
  "Name": "Chassis Sensors Collection",
  "Members@odata.count": 50,
  "Members": [
    {
      "@odata.id": "/redfish/v1/Chassis/1/Sensors/Air_Temp_1",
      "@odata.type": "#Sensor.v1_6_0.Sensor",
      "Id": "Air_Temp_1",
      "Name": "Air_Temp_1",
      "Reading": 29.000,
      "ReadingUnits": "C",
      "ReadingType": "Temperature",
      "Status": {
        "State": "Enabled",
        "Health": "OK"
      },
      "Thresholds": {
        "UpperCritical": {
          "Reading": 75.000,
          "Activation": "Increasing"
        }
      }
    }
  ]
}
```

## Building

### Prerequisites

- C++17 compatible compiler (GCC 8+ or Clang 7+)
- CMake 3.16+
- Boost libraries (system, json)
- OpenSSL
- pthread

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libboost-all-dev libssl-dev
```

### Build Instructions
```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Basic Usage
```bash
# Using default parameters (BMC IP: 192.168.1.100, username: admin, password: admin)
./yinshan_monitor

# With custom parameters
./async_yinshan_monitor <BMC_IP> <USERNAME> <PASSWORD>
```

### Example
```bash
./async_yinshan_monitor 192.168.1.50 root password123
```

## Code Structure

### Core Classes

1. **YinshanMonitor**: Basic synchronous implementation
2. **AsyncYinshanMonitor**: Enhanced asynchronous implementation with timer-based periodic fetching

### Key Components

- **Session Management**: Handles Redfish session creation and token management
- **HTTPS Client**: SSL/TLS enabled HTTP client using Boost.Beast
- **Sensor Mapping**: Maps BMC sensor names to internal configurations
- **Data Processing**: Parses JSON responses and extracts sensor values

### Sensor Mapping Configuration

The system maps BMC sensor names to internal sensor configurations:

```cpp
const std::map<std::string, RedfishUrlInfo> yinshanDPUSensorCmdList = {
    {"DPU_NIC_OPT0_Temp", {"NIC_OPT0_Temp", PROPERTY_TEMP_SENSOR, "opt_temp"}},
    {"DPU_CPU_Temp", {"CPU_Temp", PROPERTY_TEMP_SENSOR, "cpu_temp"}},
    {"DPU_Cpu_Status", {"CPU_Status", PROPERTY_POWER_SENSOR, "status"}},
    // ... more mappings
};
```

## Configuration

### Fetch Interval
The default fetch interval is 30 seconds, but can be configured when creating the AsyncYinshanMonitor:

```cpp
AsyncYinshanMonitor monitor(ioc, hostBmcIp, username, password, 
                           std::chrono::seconds(60)); // 60-second interval
```

### SSL Configuration
The implementation uses SSL with certificate verification disabled for self-signed certificates commonly used in BMC systems.

## Error Handling

The system includes comprehensive error handling:

- **Network Errors**: Automatic retry with exponential backoff
- **Authentication Errors**: Session recreation on token expiration
- **JSON Parsing Errors**: Graceful handling of malformed responses
- **SSL Errors**: Proper SSL error reporting and recovery

## Monitoring Output

The system provides detailed logging:

```
Starting async BMC sensor monitoring...
Creating BMC session...
Async session created successfully
Fetching sensor data asynchronously...
Async assigned sensor CPU_Temp value: 48.0 to DPU_CPU_Temp
Async assigned sensor Air_Temp_1 value: 29.0 to DPU_NIC_OPT0_Temp
Async processing completed: matched 8 sensors
```

## Integration

To integrate this into your OpenBMC application:

1. Include the header files
2. Create an io_context for async operations
3. Instantiate AsyncYinshanMonitor with your BMC credentials
4. Call start() to begin monitoring
5. Access sensor data via getSensorData()

```cpp
#include "async_yinshan_monitor.hpp"

boost::asio::io_context ioc;
AsyncYinshanMonitor monitor(ioc, "192.168.1.100", "admin", "password");
monitor.start();

// Run in background thread
std::thread ioThread([&ioc]() { ioc.run(); });

// Access sensor data
const auto& sensorData = monitor.getSensorData();
```

## License

This project is part of the OpenBMC ecosystem and follows OpenBMC licensing guidelines.