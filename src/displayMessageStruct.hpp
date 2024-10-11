#ifndef _DISPLAY_MESSAGE_STRUCT_
#define _DISPLAY_MESSAGE_STRUCT_

struct displayMessage
{
    enum type
    {
        SYSTEM_MESSAGE,
        CO2_LEVEL,
        TEMPERATURE,
        HUMIDITY,
        CO2_HISTORY,
        HUMIDITY_HISTORY,
        TEMPERATURE_HISTORY,        
    };
    type type;
    char str[50];
    size_t sizeVal = 0;
    float floatVal = 0;
};

#endif