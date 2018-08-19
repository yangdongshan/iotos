#ifndef TIME_h
#define TIME_H

#include <typedef.h>

#define MS2TICKS(ms) ((ms * CONFIG_TICKS_PER_SEC) / 1000)

extern unsigned long g_sys_ticks;

static inline void sys_time_tick(void)
{
    g_sys_ticks++;
}

static inline tick_t get_sys_tick(void)
{
    return g_sys_ticks;
}

int msleep(unsigned int ms);

#endif // TIME_H

