# BIOS Configuration Mapper

This project provides a C implementation to map BIOS configuration data to JSON format for Redfish API compatibility.

## Overview

The code categorizes BIOS configuration options into different types:
- **CPU Configuration**: Processor settings like Hyper-Threading, Virtualization, SpeedStep, etc.
- **Memory Configuration**: Memory settings like NUMA, Sub-NUMA Clustering, Lockstep Mode, etc.
- **Memory Error Configuration**: Memory error thresholds and storm detection
- **Platform Power Configuration**: Power management and energy performance settings
- **IO Configuration**: SATA controllers, SR-IOV, and VT-d settings
- **Thermal Configuration**: Fan profile settings
- **Management Configuration**: Serial console, network sharing, and communication settings
- **Boot Configuration**: Boot mode settings

## File Structure

- `bios_config_tables.h`: Header file containing all categorized BIOS option tables
- `bios_config_mapper.c`: Main implementation with mapping logic and JSON generation
- `Makefile`: Build configuration

## How It Works

1. **Data Structure**: Each BIOS option is defined with:
   - Valid value set (possible byte values)
   - Option name (string identifier)
   - Value names (human-readable strings for each value)

2. **Mapping Logic**: 
   - Uses `i` (byte array index) to determine which BIOS option
   - Uses `modifiedValue->byteArray[i]` to determine the specific value
   - Maps through categories sequentially: CPU → Memory → Platform Power → IO → Thermal → Management → Boot

3. **JSON Output**: Generates `/var/backups/redfish/bios_future_settings.json` with:
   ```json
   {
     "@odata.type": "#Bios.v1_1_0.Bios",
     "Attributes": {
       "Intel-Virtualization-Technology": "Enabled",
       "Intel-Hyper-Threading-Tech": "Disabled",
       ...
     }
   }
   ```

## Usage

### Compilation
```bash
make
```

### Running with Example Data
```bash
sudo ./bios_config_mapper
```

### Integration in Your Code
```c
#include "bios_config_tables.h"

// Your byte array with BIOS settings
uint8_t your_byte_array[] = {1, 0, 1, 0, ...};
uint32_t array_size = sizeof(your_byte_array);

// Process and generate JSON
int result = process_bios_configuration(your_byte_array, array_size);
```

## Byte Array Mapping

The byte array index corresponds to options in this order:

**CPU (indices 0-7):**
- 0: Intel-Hyper-Threading-Tech
- 1: Intel-Virtualization-Technology
- 2: Enhanced-Intel-SpeedStep-Technology
- 3: Intel-Turbo-Boost
- 4: Package-C-State-limit
- 5: C1E-State
- 6: C6-State
- 7: CPU-T-State

**Memory (indices 8-12):**
- 8: NUMA-Optimized
- 9: Sub-NUMA-Clustering
- 10: Lockstep-Mode
- 11: Mirror-Mode
- 12: Memory-Rank-Sparing

**Memory Error (indices 13-14):**
- 13: Memory-CE-Accumulation-Threshold
- 14: Memory-CE-Storm-Threshold

**Platform Power (indices 15-16):**
- 15: Platform-Energy-performance-mode
- 16: Power-restore-on-AC-Loss

**IO (indices 17-22):**
- 17: Intel-VT-for-Directed-IO
- 18: SATA-controller-1
- 19: SATA-controller-1-mode
- 20: SATA-controller-2
- 21: SATA-controller-2-mode
- 22: SR-IOV-Support

**Thermal (index 23):**
- 23: Fan-profile

**Management (indices 24-31):**
- 24: Sharelink-Network
- 25: Serial-port-console-redirection
- 26: Terminal-type
- 27: Bit-Rate
- 28: Flow-control
- 29: Data-bits
- 30: Parity
- 31: Stop-bits

**Boot (index 32):**
- 32: Boot-mode

## Requirements

- GCC compiler with C99 support
- sudo privileges for writing to `/var/backups/redfish/`
- Linux environment with standard system headers

## Notes

- The program requires sudo privileges to write to the system directory
- Invalid byte values will result in "Invalid" entries in the debug output
- The JSON file follows Redfish BIOS schema format