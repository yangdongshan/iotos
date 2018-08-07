#ifndef PORT_H
#define PORT_H

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


void arch_init_context_frame(struct context_frame *cf, addr_t * ip, void *arg);



static inline void irq_enable()
{
    __asm__ __volatile__ ("cpsie i\n");
}

static inline void irq_disable()
{
    __asm__ __volatile__ ("cpsid i\n");
}

static inline irqstate_t irq_disable_state_save(void)
{
    irqstate_t state;

    __asm__ __volatile__ (
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
    __asm__ __volatile__ (
            "tst %0, #1\n"
            "bne.n 1f\n"
            "cpsie i\n"
            "1:\n"
            :
            : "r" (state)
            : "memory"
            );
}

void arch_context_switch_to(unsigned char* sp);

void arch_context_switch(unsigned char *new_sp, unsigned char *old_sp);

void arch_init_context_frame(struct context_frame *cf, addr_t *entry, void *arg);

#endif // PORT_H
