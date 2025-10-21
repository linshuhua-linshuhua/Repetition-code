#include "fru_parser.h"
#include <iostream>

/**
 * Example demonstrating how to use the FRUParser class
 * in an OpenBMC application
 */

void demonstrateBasicUsage() {
    std::cout << "=== Basic Usage Example ===" << std::endl;
    
    FRUParser parser;
    
    // Extract Product Name from ipmitool fru print 0
    if (parser.extractProductName()) {
        std::cout << "Product Name successfully extracted!" << std::endl;
        std::cout << "Product Name: " << parser.getProductName() << std::endl;
        
        // You can now use the product name for further processing
        std::string productName = parser.getProductName();
        
        // Example: Log the product name
        std::cout << "Logging product name: " << productName << std::endl;
        
        // Example: Use in configuration
        std::cout << "Using product name for system configuration..." << std::endl;
        
    } else {
        std::cout << "Failed to extract Product Name" << std::endl;
        std::cout << "This could be due to:" << std::endl;
        std::cout << "  - ipmitool not available" << std::endl;
        std::cout << "  - FRU data not accessible" << std::endl;
        std::cout << "  - Product Name field not present in FRU" << std::endl;
    }
}

void demonstrateErrorHandling() {
    std::cout << "\n=== Error Handling Example ===" << std::endl;
    
    FRUParser parser;
    
    try {
        if (parser.extractProductName()) {
            if (parser.hasProductName()) {
                std::cout << "Product Name: " << parser.getProductName() << std::endl;
                
                // Validate product name
                const std::string& productName = parser.getProductName();
                if (productName.empty()) {
                    std::cout << "Warning: Product Name is empty" << std::endl;
                } else if (productName.length() > 100) {
                    std::cout << "Warning: Product Name is unusually long" << std::endl;
                } else {
                    std::cout << "Product Name validation passed" << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

void demonstrateIntegrationWithOpenBMC() {
    std::cout << "\n=== OpenBMC Integration Example ===" << std::endl;
    
    FRUParser parser;
    
    // In a real OpenBMC application, you might use this in:
    // 1. System initialization
    // 2. Hardware inventory collection
    // 3. Configuration management
    // 4. Logging and monitoring
    
    if (parser.extractProductName()) {
        const std::string& productName = parser.getProductName();
        
        // Example: Store in system configuration
        std::cout << "Storing product name in system config: " << productName << std::endl;
        
        // Example: Use for hardware-specific behavior
        if (productName.find("MoHe") != std::string::npos) {
            std::cout << "Detected MoHe hardware, applying specific configuration..." << std::endl;
        }
        
        // Example: Report to management interface
        std::cout << "Reporting to BMC management interface: " << productName << std::endl;
        
        // Example: Log for diagnostics
        std::cout << "Diagnostic log: Product Name = " << productName << std::endl;
    }
}

int main() {
    std::cout << "FRU Parser Usage Examples for OpenBMC" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    demonstrateBasicUsage();
    demonstrateErrorHandling();
    demonstrateIntegrationWithOpenBMC();
    
    return 0;
}