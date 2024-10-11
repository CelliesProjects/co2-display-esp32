#ifndef _FORECAST_T_
#define _FORECAST_T_

struct forecast_t
{
    time_t time;
    float temp;
    char icon[32];
};

#endif
