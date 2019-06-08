#ifndef __TICK_H
#define __TICK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <list.h>
#include <typedef.h>
#include <task.h>

#define WAIT_FOREVER     ((tick_t)-1)
#define WAIT_NONE        ((tick_t)0)

#define MS2TICKS(ms) ((ms * CONFIG_TICKS_PER_SEC) / 1000)

extern tick_t g_sys_ticks;

static inline void sys_tick_inc(void)
{
    g_sys_ticks++;
}

static inline tick_t get_sys_tick(void)
{
    return g_sys_ticks;
}


void tick_init_early(void);

void tick_list_insert(task_t *task);

void tick_update(void);


#ifdef __cplusplus
}
#endif

#endif // __TICK_H
