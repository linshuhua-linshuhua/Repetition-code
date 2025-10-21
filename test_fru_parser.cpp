#include "fru_parser.h"
#include <iostream>
#include <cassert>

// Test function to simulate FRU output parsing without actually running ipmitool
void testParseProductName() {
    std::cout << "Running unit tests..." << std::endl;
    
    // Create a mock FRU output similar to the provided example
    std::string mockFruOutput = R"( Chassis Type          : Rack Mount Chassis
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
 Product Extra         : aaaa)";
    
    // Test the parsing logic directly
    class TestFRUParser : public FRUParser {
    public:
        bool testParseProductName(const std::string& fruOutput) {
            return parseProductName(fruOutput);
        }
        
        // Make parseProductName accessible for testing
        using FRUParser::parseProductName;
    };
    
    TestFRUParser testParser;
    
    // Test 1: Valid FRU output with Product Name
    if (testParser.testParseProductName(mockFruOutput)) {
        std::cout << "✓ Test 1 passed: Product Name extracted: " << testParser.getProductName() << std::endl;
        assert(testParser.getProductName() == "MoHe-S5N2");
    } else {
        std::cout << "✗ Test 1 failed: Could not extract Product Name" << std::endl;
        return;
    }
    
    // Test 2: FRU output without Product Name
    std::string mockFruOutputNoProduct = R"( Chassis Type          : Rack Mount Chassis
 Chassis Part Number   : 1111
 Chassis Serial        : 24004567
 Board Mfg Date        : Thu Jan 18 14:42:00 2024 CST
 Board Mfg             : Nettrix-MH)";
    
    TestFRUParser testParser2;
    if (!testParser2.testParseProductName(mockFruOutputNoProduct)) {
        std::cout << "✓ Test 2 passed: Correctly identified missing Product Name" << std::endl;
        assert(!testParser2.hasProductName());
    } else {
        std::cout << "✗ Test 2 failed: Should not have found Product Name" << std::endl;
        return;
    }
    
    // Test 3: Different formatting
    std::string mockFruOutputDifferentFormat = R"(Product Name:TestProduct123)";
    
    TestFRUParser testParser3;
    if (testParser3.testParseProductName(mockFruOutputDifferentFormat)) {
        std::cout << "✓ Test 3 passed: Product Name extracted with different format: " << testParser3.getProductName() << std::endl;
        assert(testParser3.getProductName() == "TestProduct123");
    } else {
        std::cout << "✗ Test 3 failed: Could not extract Product Name with different format" << std::endl;
        return;
    }
    
    std::cout << "All unit tests passed!" << std::endl;
}

int main() {
    std::cout << "=== FRU Parser Test Program ===" << std::endl;
    
    // Run unit tests first
    testParseProductName();
    
    std::cout << "\n=== Live Test with ipmitool ===" << std::endl;
    
    // Test with actual ipmitool command (if available)
    FRUParser parser;
    
    std::cout << "Attempting to execute ipmitool fru print 0 command..." << std::endl;
    
    if (parser.extractProductName()) {
        std::cout << "\nSuccess! Product Name extracted: " << parser.getProductName() << std::endl;
    } else {
        std::cout << "\nNote: ipmitool command failed (this is expected if not running on BMC hardware)" << std::endl;
        std::cout << "The unit tests above demonstrate that the parsing logic works correctly." << std::endl;
    }
    
    return 0;
}