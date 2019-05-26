#ifndef __TICK_H
#define __TICK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <list.h>
#include <typedef.h>

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


typedef void (*ticker_cb)(void *arg);

typedef struct {
    struct list_node node;
    tick_t timeout;
    ticker_cb ticker_handler;
    void *arg;
} ticker_t;

void tick_init_early(void);

void tick_update(void);

void tick_list_wait(ticker_t *ticker,
                    tick_t delay_ticks,
                    ticker_cb *handler,
                    void *arg);

int tick_cancel(ticker_t *ticker);

#ifdef __cplusplus
}
#endif

#endif // __TICK_H
