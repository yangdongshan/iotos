#ifndef __PORT_H
#define __PORT_H

#include <typedef.h>

struct context_frame {
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t ip;
    uint32_t xpsr;
};

static inline void irq_enable()
{
    __asm__ __volatile__ ("cpsie i:::memory\n");
}

static inline void irq_disable()
{
   // __asm__ __volatile__ ("cpsid i:::memory\n");
}

static inline irqstate_t irq_state_save(void)
{
    irqstate_t state;

    __asm__ __volatile__ (
            "mrs %0, primask\n"
            : "=r" (state)
            );

    return state;
}

static inline void irq_state_restore(irqstate_t state)
{
    __asm__ __volatile__ (
            "tst %0, #1\n"
            "bne.n 1f\n"
            "cpsie i\n"
            "nop\n"
            "1:\n"
            :
            : "r" (state)
            : "memory"
            );
}

static inline irqstate_t arch_enter_critical_section(void)
{
	irqstate_t state;

	state = irq_state_save();
	irq_disable();

    return state;
}

static inline void arch_leave_critical_section(irqstate_t state)
{
    irq_state_restore(state);
}


void arch_start_first_task(void);

void arch_context_switch(void);

void arch_init_context_frame(struct context_frame *cf,
                             addr_t * ip, void *arg,
                             addr_t *task_grave);

#endif // __PORT_H

