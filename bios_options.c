#include "bios_options.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Function implementation
static int setBiosOptionOutOfBand(T_REQ_SET_BIOS_OPTION* reqSetBiosOption,
                                  T_SINGLE_BIOS_OPTION_DATA* currentValue,
                                  T_SINGLE_BIOS_OPTION_DATA* modifiedValue)
{
    if (reqSetBiosOption == NULL || currentValue == NULL || 
        modifiedValue == NULL)
    {
        return -1;
    }

    modifiedValue->validFlag = E_BOVF_VALID;

    if (reqSetBiosOption->dataType == E_BOT_DEBUG)
    {
        memcpy(modifiedValue->byteArray, reqSetBiosOption->data,
               sizeof(modifiedValue->byteArray));

        if (currentValue->validFlag == E_BOVF_VALID &&
            memcmp(currentValue->byteArray, modifiedValue->byteArray,
                   sizeof(currentValue->byteArray)) == 0)
        {
            modifiedValue->byteArray[0] &= ~E_SBC_BIOS_OPTION_PENDING_MASK;
        }
        else
        {
            modifiedValue->byteArray[0] |= E_SBC_BIOS_OPTION_PENDING_MASK;
        }
    }
    else
    {
        for (unsigned int i = 0; i < SINGLE_BIOS_OPTION_LENGTH; ++i)
        {
            if (reqSetBiosOption->data[i] == E_BOVBV_NO_CHANGE)
            {
                continue;
            }

            modifiedValue->byteArray[i] = reqSetBiosOption->data[i];

            if (currentValue->validFlag == E_BOVF_VALID &&
                currentValue->byteArray[i] == modifiedValue->byteArray[i])
            {
                modifiedValue->byteArray[i] &= ~E_SBC_BIOS_OPTION_PENDING_MASK;
            }
            else
            {
                modifiedValue->byteArray[i] |= E_SBC_BIOS_OPTION_PENDING_MASK;
            }
        }
    }

    return 0;
}