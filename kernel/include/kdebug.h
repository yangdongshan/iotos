#ifndef __KDEBUG_H
#define __KDEBUG_H

#include <arch_debug.h>

#include <irq.h>

typedef int (*print_func)(const char *str, int len);

int kdebug_print(const char *fmt, ...);

#define    ERR  0
#define    WARN 1
#define    INFO 2
#define    DEBUG 3


#ifdef CONFIG_KERNEL_DEBUG

#if (CONFIG_KERNEL_DEBUG_LEVEL_ERR)
#define KERNEL_DEBUG_LEVLE ERR
#elif (CONFIG_KERNEL_DEBUG_LEVEL_INFO)
#define KERNEL_DEBUG_LEVLE INFO
#elif (CONFIG_KERNEL_DEBUG_LEVEL_WARN)
#define KERNEL_DEBUG_LEVLE WARN
#elif (CONFIG_KERNEL_DEBUG_LEVEL_DEBUG)
#define KERNEL_DEBUG_LEVLE DEBUG
#else
#define KERNEL_DEBUG_LEVLE ERR
#endif

#if (KERNEL_DEBUG_LEVLE >= DEBUG)
#define KDBG(fmt, args...) \
    do { \
            irqstate_t __state; \
            __state = enter_critical_section(); \
            kdebug_print("[DBG] "fmt, ##args); \
            leave_critical_section(__state); \
    } while (0)

#else
#define KDBG(fmt, args...)
#endif // CONFIG_KERNEL_DEBUG

#if (KERNEL_DEBUG_LEVLE >= INFO)
#define KINFO(fmt, args...) \
    do { \
            irqstate_t __state; \
            __state = enter_critical_section(); \
            kdebug_print("[INFO] "fmt, ##args); \
            leave_critical_section(__state); \
    } while (0)

#else
#define KINFO(fmt, args...)
#endif // CONFIG_KERNEL_DEBUG

#if (KERNEL_DEBUG_LEVLE >= WARN)
#define KWARN(fmt, args...) \
    do { \
            irqstate_t __state; \
            __state = enter_critical_section(); \
            kdebug_print("[WARN] "fmt, ##args); \
            leave_critical_section(__state); \
    } while (0)

#else
#define KWARN(fmt, args...)
#endif // CONFIG_KERNEL_DEBUG

#if (KERNEL_DEBUG_LEVLE >= ERR)
#define KERR(fmt, args...) \
    do { \
            irqstate_t __state; \
            __state = enter_critical_section(); \
            kdebug_print("[ERR] "fmt, ##args); \
            leave_critical_section(__state); \
    } while (0)

#else
#define KERR(fmt, args...)
#endif // CONFIG_KERNEL_DEBUG

#else

#define KDBG(fmt, args...)
#define KINFO(fmt, args...)
#define KWARN(fmt, args...)
#define KERR(fmt, args...)

#endif

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
