#ifndef FRU_PARSER_H
#define FRU_PARSER_H

#include <string>

/**
 * FRUParser class for extracting Product Name from ipmitool fru print output
 * 
 * This class provides functionality to:
 * 1. Execute "ipmitool fru print 0" command
 * 2. Parse the output to find Product Name
 * 3. Store the Product Name in a member variable
 */
class FRUParser {
private:
    std::string productName;
    
    /**
     * Execute a system command and return its output
     * @param command The command to execute
     * @return The command output as string
     * @throws std::runtime_error if command execution fails
     */
    std::string executeCommand(const std::string& command);

protected:
    /**
     * Parse FRU output and extract Product Name
     * @param fruOutput The raw FRU output string
     * @return true if Product Name was found and extracted, false otherwise
     */
    bool parseProductName(const std::string& fruOutput);

public:
    /**
     * Execute ipmitool fru print 0 command and extract Product Name
     * @return true if Product Name was successfully extracted, false otherwise
     */
    bool extractProductName();
    
    /**
     * Get the extracted Product Name
     * @return const reference to the Product Name string
     */
    const std::string& getProductName() const;
    
    /**
     * Check if Product Name was found
     * @return true if Product Name is available, false otherwise
     */
    bool hasProductName() const;
};

#endif // FRU_PARSER_H