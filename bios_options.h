#ifndef BIOS_OPTIONS_H
#define BIOS_OPTIONS_H

#include <stdint.h>

// Constants
#define SINGLE_BIOS_OPTION_LENGTH 16
#define E_SBC_BIOS_OPTION_PENDING_MASK 0x80

// Enum for BIOS option valid flags
typedef enum {
    E_BOVF_INVALID = 0,
    E_BOVF_VALID = 1
} E_BIOS_OPTION_VALID_FLAG;

// Enum for BIOS option types
typedef enum {
    E_BOT_NORMAL = 0,
    E_BOT_DEBUG = 1
} E_BIOS_OPTION_TYPE;

// Enum for BIOS option byte values
typedef enum {
    E_BOVBV_NO_CHANGE = 0xFF
} E_BIOS_OPTION_BYTE_VALUE;

// Enum for check modes
typedef enum {
    E_BOCM_VALID_DATASET,
    E_BOCM_INVALID_BITSET_MASK
} E_CHECK_MODE;

// Struct for single BIOS option data
typedef struct {
    E_BIOS_OPTION_VALID_FLAG validFlag;
    unsigned char byteArray[SINGLE_BIOS_OPTION_LENGTH];
} T_SINGLE_BIOS_OPTION_DATA;

// Struct for BIOS option request
typedef struct {
    E_BIOS_OPTION_TYPE dataType;
    unsigned char data[SINGLE_BIOS_OPTION_LENGTH];
} T_REQ_SET_BIOS_OPTION;

// Struct for valid byte set
typedef struct {
    unsigned char count;
    unsigned char array[SINGLE_BIOS_OPTION_LENGTH];
} T_VALID_BYTE_SET;

// Struct for invalid bit mask
typedef struct {
    unsigned char mask;
} T_INVALID_BIT_MASK;

// Union for check data
typedef union {
    T_VALID_BYTE_SET validByteSet;
    T_INVALID_BIT_MASK invalidBitMask;
} U_CHECK_DATA;

// Struct for check byte
typedef struct {
    E_CHECK_MODE mode;
    U_CHECK_DATA data;
} T_CHECK_BYTE;

// Function prototypes
static int setBiosOptionOutOfBand(T_REQ_SET_BIOS_OPTION* reqSetBiosOption,
                                  T_SINGLE_BIOS_OPTION_DATA* currentValue,
                                  T_SINGLE_BIOS_OPTION_DATA* modifiedValue);

#endif // BIOS_OPTIONS_H