#ifndef TIME_h
#define TIME_H

#include <typedef.h>

#define MS2TICKS(ms) ((ms * CONFIG_TICKS_PER_SEC) / 1000)

void sys_time_tick(void);

tick_t get_sys_tick(void);

#endif // TIME_H

