#include "fru_parser.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <regex>
#include <cstdio>
#include <array>

std::string FRUParser::executeCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("Failed to execute command: " + command);
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return result;
}

bool FRUParser::parseProductName(const std::string& fruOutput) {
    std::istringstream stream(fruOutput);
    std::string line;
    
    // Regular expression to match "Product Name" line
    // Pattern: optional whitespace + "Product Name" + optional whitespace + ":" + optional whitespace + value
    std::regex productNameRegex(R"(\s*Product\s+Name\s*:\s*(.+))");
    std::smatch match;
    
    while (std::getline(stream, line)) {
        if (std::regex_search(line, match, productNameRegex)) {
            productName = match[1].str();
            // Remove trailing whitespace
            productName.erase(productName.find_last_not_of(" \t\r\n") + 1);
            return true;
        }
    }
    
    return false;
}

bool FRUParser::extractProductName() {
    try {
        std::string command = "ipmitool fru print 0";
        std::string fruOutput = executeCommand(command);
        
        if (fruOutput.empty()) {
            std::cerr << "Error: No output from ipmitool command" << std::endl;
            return false;
        }
        
        std::cout << "FRU Output:\n" << fruOutput << std::endl;
        
        if (parseProductName(fruOutput)) {
            std::cout << "Product Name found: " << productName << std::endl;
            return true;
        } else {
            std::cout << "Product Name not found in FRU output" << std::endl;
            return false;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

const std::string& FRUParser::getProductName() const {
    return productName;
}

bool FRUParser::hasProductName() const {
    return !productName.empty();
}