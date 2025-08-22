#ifndef BIOS_CONFIG_TABLES_H
#define BIOS_CONFIG_TABLES_H

#include <stdint.h>

// Define the structure for BIOS option checking
typedef enum {
    E_BOCM_VALID_DATASET,
    E_BOCM_INVALID_BITSET_MASK
} E_BOCM_TYPE;

typedef struct {
    uint8_t count;
    uint8_t values[10];  // Maximum values array
} T_VALUE_SET;

typedef struct {
    E_BOCM_TYPE type;
    T_VALUE_SET value_set;
    char option_name[64];
    char value_names[10][32];  // String names for each value
} T_CHECK_BYTE;

#define SINGLE_BIOS_OPTION_LENGTH 16

// CPU Configuration Options
T_CHECK_BYTE g_checkCpuOption[8] = {
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Intel-Hyper-Threading-Tech", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Intel-Virtualization-Technology", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Enhanced-Intel-SpeedStep-Technology", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Intel-Turbo-Boost", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {6, {0, 1, 2, 3, 4, 5}}, "Package-C-State-limit", {"C0/C1-state", "C2-state", "C6-non-retention-state", "C6-retention-state", "No-Limit", "Auto"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "C1E-State", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "C6-State", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "CPU-T-State", {"Disable", "Enable"}}
};

// Memory Configuration Options
T_CHECK_BYTE g_checkMemoryOption[5] = {
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "NUMA-Optimized", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {4, {0, 1, 2, 3}}, "Sub-NUMA-Clustering", {"Disable", "SNC2", "SNC4", "Auto"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Lockstep-Mode", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Mirror-Mode", {"Disable", "1LM"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Memory-Rank-Sparing", {"Disable", "Enable"}}
};

// Memory Error Configuration Options
T_CHECK_BYTE g_checkMemoryErrorOption[2] = {
    {E_BOCM_VALID_DATASET, {8, {0, 1, 2, 3, 4, 5, 6, 7}}, "Memory-CE-Accumulation-Threshold", {"Disable", "1200", "1", "500", "1000", "2000", "5000", "10000"}},
    {E_BOCM_VALID_DATASET, {5, {0, 1, 2, 3, 4}}, "Memory-CE-Storm-Threshold", {"Disable", "120", "60", "240", "1200"}}
};

// Platform Power Configuration Options
T_CHECK_BYTE g_checkPlatformPowerOption[2] = {
    {E_BOCM_VALID_DATASET, {4, {0, 1, 2, 3}}, "Platform-Energy-performance-mode", {"Perf", "Balanced-Perf", "Balanced-Power", "Power"}},
    {E_BOCM_VALID_DATASET, {4, {0, 1, 2, 3}}, "Power-restore-on-AC-Loss", {"Power-off", "Last-state", "Power-on", "No-change"}}
};

// IO Configuration Options
T_CHECK_BYTE g_checkIOOption[6] = {
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Intel-VT-for-Directed-IO", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "SATA-controller-1", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "SATA-controller-1-mode", {"AHCI", "RAID"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "SATA-controller-2", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "SATA-controller-2-mode", {"AHCI", "RAID"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "SR-IOV-Support", {"Disable", "Enable"}}
};

// Thermal Configuration Options
T_CHECK_BYTE g_checkThermalOption[1] = {
    {E_BOCM_VALID_DATASET, {3, {0, 1, 2}}, "Fan-profile", {"Acoustic", "Performance", "Automatic"}}
};

// Management Configuration Options
T_CHECK_BYTE g_checkManagementOption[8] = {
    {E_BOCM_VALID_DATASET, {3, {0, 1, 2}}, "Sharelink-Network", {"Disable", "Enable", "Auto"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Serial-port-console-redirection", {"Disable", "Enable"}},
    {E_BOCM_VALID_DATASET, {4, {0, 1, 2, 3}}, "Terminal-type", {"VT100", "VT100+", "VT-UTF8", "ANSI"}},
    {E_BOCM_VALID_DATASET, {5, {0, 1, 2, 3, 4}}, "Bit-Rate", {"9600", "19200", "38400", "57600", "115200"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Flow-control", {"NONE", "Hardware"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Data-bits", {"7", "8"}},
    {E_BOCM_VALID_DATASET, {5, {0, 1, 2, 3, 4}}, "Parity", {"NONE", "Even", "Odd", "Mark", "Space"}},
    {E_BOCM_VALID_DATASET, {2, {0, 1}}, "Stop-bits", {"1", "2"}}
};

// Boot Configuration Options
T_CHECK_BYTE g_checkBootOption[1] = {
    {E_BOCM_VALID_DATASET, {3, {0, 1, 2}}, "Boot-mode", {"UEFI-and-Legacy", "Legacy-only", "UEFI-only"}}
};

// Category enumeration for mapping
typedef enum {
    CATEGORY_CPU = 0,
    CATEGORY_MEMORY = 1,
    CATEGORY_MEMORY_ERROR = 2,
    CATEGORY_PLATFORM_POWER = 3,
    CATEGORY_IO = 4,
    CATEGORY_THERMAL = 5,
    CATEGORY_MANAGEMENT = 6,
    CATEGORY_BOOT = 7,
    CATEGORY_MAX
} E_BIOS_CATEGORY;

// Category information structure
typedef struct {
    T_CHECK_BYTE* options;
    uint8_t option_count;
    char category_name[32];
} T_CATEGORY_INFO;

// Category mapping table
T_CATEGORY_INFO g_categoryTable[CATEGORY_MAX] = {
    {g_checkCpuOption, 8, "CPU"},
    {g_checkMemoryOption, 5, "Memory"},
    {g_checkMemoryErrorOption, 2, "Memory-Error"},
    {g_checkPlatformPowerOption, 2, "Platform-Power"},
    {g_checkIOOption, 6, "IO"},
    {g_checkThermalOption, 1, "Thermal"},
    {g_checkManagementOption, 8, "Management"},
    {g_checkBootOption, 1, "Boot"}
};

#endif // BIOS_CONFIG_TABLES_H