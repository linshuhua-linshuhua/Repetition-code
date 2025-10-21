# OpenBMC FRU Parser - Product Name Extraction

This project provides a C++ implementation for OpenBMC to execute the `ipmitool fru print 0` command and extract the Product Name from the FRU (Field Replaceable Unit) data.

## Overview

The FRU Parser executes the `ipmitool fru print 0` command, parses the output, and extracts the Product Name field if present. This is useful for OpenBMC applications that need to identify hardware components programmatically.

## Features

- ✅ Execute `ipmitool fru print 0` command
- ✅ Parse FRU output using regex pattern matching
- ✅ Extract Product Name and store in variable
- ✅ Robust error handling
- ✅ Comprehensive unit tests
- ✅ Modular design with header/implementation separation
- ✅ Example usage code

## Files Structure

```
├── fru_parser.h              # Header file with class declaration
├── fru_parser_impl.cpp       # Implementation file
├── fru_parser.cpp            # Single-file version (standalone)
├── test_fru_parser.cpp       # Unit tests and live testing
├── example_usage.cpp         # Usage examples for OpenBMC integration
├── Makefile                  # Build configuration
└── README_FRU_Parser.md      # This documentation
```

## Quick Start

### 1. Build the project

```bash
make all
```

### 2. Run tests

```bash
make test
# or directly:
./test_fru_parser
```

### 3. Run examples

```bash
./example_usage
```

## Usage

### Basic Usage

```cpp
#include "fru_parser.h"
#include <iostream>

int main() {
    FRUParser parser;
    
    if (parser.extractProductName()) {
        std::cout << "Product Name: " << parser.getProductName() << std::endl;
    } else {
        std::cout << "Failed to extract Product Name" << std::endl;
    }
    
    return 0;
}
```

### Advanced Usage with Error Handling

```cpp
#include "fru_parser.h"
#include <iostream>

int main() {
    FRUParser parser;
    
    try {
        if (parser.extractProductName()) {
            if (parser.hasProductName()) {
                const std::string& productName = parser.getProductName();
                std::cout << "Successfully extracted: " << productName << std::endl;
                
                // Use the product name for your application logic
                // e.g., hardware-specific configuration, logging, etc.
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
```

## API Reference

### Class: FRUParser

#### Public Methods

- `bool extractProductName()`: Execute ipmitool command and extract Product Name
  - Returns: `true` if successful, `false` otherwise

- `const std::string& getProductName() const`: Get the extracted Product Name
  - Returns: Reference to the Product Name string

- `bool hasProductName() const`: Check if Product Name was found
  - Returns: `true` if Product Name is available, `false` otherwise

## Expected FRU Output Format

The parser expects FRU output in the following format:

```
 Chassis Type          : Rack Mount Chassis
 Chassis Part Number   : 1111
 Chassis Serial        : 24004567
 Board Mfg Date        : Thu Jan 18 14:42:00 2024 CST
 Board Mfg             : Nettrix-MH
 Board Product         : MoHe
 Board Serial          : 2400
 Board Part Number     : 2400234567
 Product Manufacturer  : Nettrix-CRB
 Product Name          : MoHe-S5N2
 Product Part Number   : 25001234
 Product Version       : BirchstreamG60
 Product Serial        : 25004567
 Product Asset Tag     : productassert
 Product Extra         : aaaa
```

The parser uses a regex pattern to match the "Product Name" line:
```regex
\s*Product\s+Name\s*:\s*(.+)
```

## Testing

The project includes comprehensive unit tests that verify:

1. ✅ Correct extraction of Product Name from valid FRU output
2. ✅ Proper handling of FRU output without Product Name
3. ✅ Support for different formatting variations
4. ✅ Error handling for command execution failures

### Test Results

```
=== FRU Parser Test Program ===
Running unit tests...
✓ Test 1 passed: Product Name extracted: MoHe-S5N2
✓ Test 2 passed: Correctly identified missing Product Name
✓ Test 3 passed: Product Name extracted with different format: TestProduct123
All unit tests passed!
```

## Integration with OpenBMC

This FRU Parser can be integrated into OpenBMC applications for:

1. **System Initialization**: Identify hardware during boot
2. **Hardware Inventory**: Collect product information for management
3. **Configuration Management**: Apply hardware-specific settings
4. **Logging and Monitoring**: Record hardware information for diagnostics

### Example Integration

```cpp
// In your OpenBMC service/daemon
FRUParser parser;
if (parser.extractProductName()) {
    const std::string& productName = parser.getProductName();
    
    // Store in system configuration
    systemConfig.setProductName(productName);
    
    // Apply hardware-specific behavior
    if (productName.find("MoHe") != std::string::npos) {
        applyMoHeConfiguration();
    }
    
    // Log for diagnostics
    logger.info("Hardware identified: " + productName);
}
```

## Requirements

- C++11 or later
- `ipmitool` installed on the system
- IPMI hardware support (for live execution)

### Installing ipmitool

```bash
# Ubuntu/Debian
sudo apt-get install ipmitool

# RHEL/CentOS
sudo yum install ipmitool

# OpenBMC
# ipmitool should be available by default
```

## Build Options

### Single File Version
```bash
g++ -std=c++11 -Wall -Wextra -O2 -o fru_parser fru_parser.cpp
```

### Modular Version
```bash
g++ -std=c++11 -Wall -Wextra -O2 -I. -o my_app my_app.cpp fru_parser_impl.cpp
```

## Error Handling

The parser handles various error conditions:

- ❌ `ipmitool` command not found
- ❌ IPMI hardware not accessible
- ❌ FRU data not available
- ❌ Product Name field not present in FRU
- ❌ Command execution failures

## Performance Considerations

- Command execution is synchronous (blocking)
- Typical execution time: 100-500ms depending on hardware
- Memory usage: Minimal (stores only the Product Name string)
- Consider caching results if called frequently

## Limitations

- Requires `ipmitool` to be installed and accessible
- Depends on IPMI hardware support
- Synchronous execution (blocks during command execution)
- Currently supports only FRU index 0

## Future Enhancements

- Support for multiple FRU indices
- Asynchronous command execution
- Direct IPMI library integration (bypass ipmitool)
- Additional FRU field extraction
- Configuration file support

## License

This code is provided as an example implementation for OpenBMC projects.

## Contributing

Feel free to submit issues and enhancement requests!