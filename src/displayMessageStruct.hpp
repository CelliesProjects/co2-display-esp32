#ifndef _DISPLAY_MESSAGE_STRUCT_

struct displayMessage
{
    enum type
    {
        SYSTEM_MESSAGE,
        CO2_LEVEL,
    };
    type type;
    char str[50];
    size_t value1 = 0;
    size_t value2 = 0;
};

#endif