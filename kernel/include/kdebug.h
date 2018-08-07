#ifndef __KDEBUG_H
#define __KDEBUG_H

#include <arch_debug.h>

#include <irq.h>

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

#ifdef CONFIG_KERNEL_DEBUG

#ifdef CONFIG_KERNEL_DEBUG_LEVEL_ERR
#define KERNEL_DEBUG_LEVLE ERR
#elif CONFIG_KERNEL_DEBUG_LEVEL_WARN
#define KERNEL_DEBUG_LEVLE WARN
#elif CONFIG_KERNEL_DEBUG_LEVEL_INFO
#define KERNEL_DEBUG_LEVLE INFO
#elif CONFIG_KERNEL_DEBUG_LEVEL_DEBUG
#define KERNEL_DEBUG_LEVLE DEBUG
#else
#define KERNEL_DEBUG_LEVLE ERR
#endif

//kdebug_print("["#level"]: "#fmt, ##args);
#define KDBG(level, fmt, args...) \
    do { \
        if (level <= KERNEL_DEBUG_LEVLE) { \
            irqstate_t __state; \
            __state = enter_critical_section(); \
            kdebug_print("["#level"] "fmt, ##args); \
            leave_critical_section(__state); \
        } \
    } while (0)

#else
#define KDBG(level, fmt, args...)
#endif // CONFIG_KERNEL_DEBUG

#ifdef CONFIG_KERNEL_ASSERT

#define KASSERT(cond) \
    do { \
        if (!(cond)) { \
            kdebug_print("[%s:%d] assert (%s) failed\n", __FILE__, __LINE__, #cond); \
            while (1); \
        } \
    } while (0)

#else
#define KASSERT(cond)
#endif // CONFIG_KERNEL_ASSERT

#endif // __KDEBUG_H
