#ifndef TIME_h
#define TIME_H

#include <typedef.h>

#define MS2TICKS(ms) ((ms * CONFIG_TICKS_PER_SEC) / 1000)

extern unsigned long gticks;

static inline void sys_time_tick(void)
{
    gticks++;
}

static inline tick_t get_sys_tick(void)
{
    return gticks;
}

#endif // TIME_H

