#ifndef __KDEBUG_H
#define __KDEBUG_H

#include <types.h>
#include <arch_debug.h>

typedef int (*print_func)(const char *str, int len);

int kdebug_print(const char *fmt, ...);

enum {
    ERR = 0,
    WARN,
    INFO,
    DEBUG,
};

#ifndef CONFIG_KERNEL_DEBUG_LEVLE
#define CONFIG_KERNEL_DEBUG_LEVLE DEBUG
#endif

#define KDBG_LEVEL CONFIG_KERNEL_DEBUG_LEVLE

#define KDBG(level, fmt, args...) \
    do { \
        if (level <= KDBG_LEVEL) { \
            kdebug_print("["#level"]:"#fmt, ##args); \
        } \
    } while (0)

#define kassert(cond) \
    do { \
    } while (0)

#endif // __KDEBUG_H
