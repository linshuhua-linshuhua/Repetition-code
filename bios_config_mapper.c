#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "bios_config_tables.h"

// Structure to hold modified values
typedef struct {
    uint8_t* byteArray;
    uint32_t arraySize;
} T_MODIFIED_VALUE;

// Function to create directory if it doesn't exist
int create_directory_if_not_exists(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            if (errno != EEXIST) {
                perror("mkdir");
                return -1;
            }
        }
    }
    return 0;
}

// Function to map category and option index to BIOS setting
const char* get_bios_setting_value(uint8_t category_index, uint8_t option_index, uint8_t value_index) {
    if (category_index >= CATEGORY_MAX) {
        return NULL;
    }
    
    T_CATEGORY_INFO* category = &g_categoryTable[category_index];
    
    if (option_index >= category->option_count) {
        return NULL;
    }
    
    T_CHECK_BYTE* option = &category->options[option_index];
    
    if (value_index >= option->value_set.count) {
        return NULL;
    }
    
    // Find the actual value in the value set
    for (int i = 0; i < option->value_set.count; i++) {
        if (option->value_set.values[i] == value_index) {
            return option->value_names[i];
        }
    }
    
    return NULL;
}

// Function to get option name
const char* get_bios_option_name(uint8_t category_index, uint8_t option_index) {
    if (category_index >= CATEGORY_MAX) {
        return NULL;
    }
    
    T_CATEGORY_INFO* category = &g_categoryTable[category_index];
    
    if (option_index >= category->option_count) {
        return NULL;
    }
    
    return category->options[option_index].option_name;
}

// Function to write BIOS settings to JSON file
int write_bios_settings_to_json(T_MODIFIED_VALUE* modifiedValue) {
    // Create directory structure
    if (create_directory_if_not_exists("/var") != 0) return -1;
    if (create_directory_if_not_exists("/var/backups") != 0) return -1;
    if (create_directory_if_not_exists("/var/backups/redfish") != 0) return -1;
    
    FILE* fp = fopen("/var/backups/redfish/bios_future_settings.json", "w");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"@odata.type\": \"#Bios.v1_1_0.Bios\",\n");
    fprintf(fp, "  \"Attributes\": {\n");
    
    int first_attribute = 1;
    uint32_t byte_index = 0;
    
    // Iterate through all categories
    for (uint8_t category = 0; category < CATEGORY_MAX && byte_index < modifiedValue->arraySize; category++) {
        T_CATEGORY_INFO* cat_info = &g_categoryTable[category];
        
        // Iterate through all options in this category
        for (uint8_t option = 0; option < cat_info->option_count && byte_index < modifiedValue->arraySize; option++) {
            uint8_t value_index = modifiedValue->byteArray[byte_index];
            
            const char* option_name = get_bios_option_name(category, option);
            const char* setting_value = get_bios_setting_value(category, option, value_index);
            
            if (option_name && setting_value) {
                if (!first_attribute) {
                    fprintf(fp, ",\n");
                }
                fprintf(fp, "    \"%s\": \"%s\"", option_name, setting_value);
                first_attribute = 0;
            }
            
            byte_index++;
        }
    }
    
    fprintf(fp, "\n  }\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

// Example usage function
int process_bios_configuration(uint8_t* byte_array, uint32_t array_size) {
    T_MODIFIED_VALUE modifiedValue = {
        .byteArray = byte_array,
        .arraySize = array_size
    };
    
    printf("Processing BIOS configuration with %d bytes...\n", array_size);
    
    // Debug output - show what we're mapping
    uint32_t byte_index = 0;
    for (uint8_t category = 0; category < CATEGORY_MAX && byte_index < array_size; category++) {
        T_CATEGORY_INFO* cat_info = &g_categoryTable[category];
        printf("\n%s Configuration:\n", cat_info->category_name);
        
        for (uint8_t option = 0; option < cat_info->option_count && byte_index < array_size; option++) {
            uint8_t value_index = byte_array[byte_index];
            const char* option_name = get_bios_option_name(category, option);
            const char* setting_value = get_bios_setting_value(category, option, value_index);
            
            printf("  [%d] %s = %s (value_index: %d)\n", 
                   byte_index, option_name ? option_name : "Unknown", 
                   setting_value ? setting_value : "Invalid", value_index);
            
            byte_index++;
        }
    }
    
    // Write to JSON file
    int result = write_bios_settings_to_json(&modifiedValue);
    if (result == 0) {
        printf("\nBIOS settings successfully written to /var/backups/redfish/bios_future_settings.json\n");
    } else {
        printf("\nError writing BIOS settings to JSON file\n");
    }
    
    return result;
}

// Main function for testing
int main() {
    // Example byte array - you would replace this with your actual data
    uint8_t example_bytes[] = {
        1,  // CPU: Intel-Hyper-Threading-Tech = Enable
        0,  // CPU: Intel-Virtualization-Technology = Disable
        1,  // CPU: Enhanced-Intel-SpeedStep-Technology = Enable
        1,  // CPU: Intel-Turbo-Boost = Enable
        5,  // CPU: Package-C-State-limit = Auto
        0,  // CPU: C1E-State = Disable
        1,  // CPU: C6-State = Enable
        0,  // CPU: CPU-T-State = Disable
        1,  // Memory: NUMA-Optimized = Enable
        3,  // Memory: Sub-NUMA-Clustering = Auto
        0,  // Memory: Lockstep-Mode = Disable
        1,  // Memory: Mirror-Mode = 1LM
        0,  // Memory: Memory-Rank-Sparing = Disable
        1,  // Memory-Error: Memory-CE-Accumulation-Threshold = 1200
        0,  // Memory-Error: Memory-CE-Storm-Threshold = Disable
        2,  // Platform-Power: Platform-Energy-performance-mode = Balanced-Power
        1,  // Platform-Power: Power-restore-on-AC-Loss = Last-state
        1,  // IO: Intel-VT-for-Directed-IO = Enable
        1,  // IO: SATA-controller-1 = Enable
        0,  // IO: SATA-controller-1-mode = AHCI
        0,  // IO: SATA-controller-2 = Disable
        0,  // IO: SATA-controller-2-mode = AHCI
        1,  // IO: SR-IOV-Support = Enable
        1,  // Thermal: Fan-profile = Performance
        2,  // Management: Sharelink-Network = Auto
        1,  // Management: Serial-port-console-redirection = Enable
        0,  // Management: Terminal-type = VT100
        4,  // Management: Bit-Rate = 115200
        0,  // Management: Flow-control = NONE
        1,  // Management: Data-bits = 8
        0,  // Management: Parity = NONE
        0,  // Management: Stop-bits = 1
        0   // Boot: Boot-mode = UEFI-and-Legacy
    };
    
    return process_bios_configuration(example_bytes, sizeof(example_bytes));
}

// Function to get total number of options across all categories
uint32_t get_total_option_count() {
    uint32_t total = 0;
    for (uint8_t category = 0; category < CATEGORY_MAX; category++) {
        total += g_categoryTable[category].option_count;
    }
    return total;
}

// Function to validate byte array size
int validate_byte_array_size(uint32_t array_size) {
    uint32_t expected_size = get_total_option_count();
    if (array_size != expected_size) {
        printf("Warning: Expected %d bytes, got %d bytes\n", expected_size, array_size);
        return 0;  // Continue processing with available data
    }
    return 1;
}