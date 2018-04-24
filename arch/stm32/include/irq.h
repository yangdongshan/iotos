#ifndef __IRQ_H
#define __IRQ_H

#include <types.h>

static inline void irq_enable()
{
    __asm__ __valotile__ ("cpsie i\n");
}

static inline void irq_disable()
{
    __asm__ __valotile__ ("cpsid i\n");
}

static inline irqstate_t irq_disable_state_save(void)
{
    irqstate_t state;

    __asm__ __valotile__ (
            "mrs %0, primask\n"
            "cpsid i\n"
            : "=r" (state)
            :
            : "memory"
            );

    return state;
}

static inline void irq_state_restore(irqstate_t state)
{
    __asm__ __valotile__ (
            "tst %0, #1\n"
            "bne.n 1f\n"
            "cpsie i\n"
            "1:\n"
            :
            : "r" (state)
            : "memory"
            );
}

#define enter_critical_section() irq_disable_state_save()

#define leave_critical_section(f) irq_state_restore(f)

#endif // __IRQ_H

