#ifndef __CONTEXT_H
#define __CONTEXT_H

#include <type_def.h>
#include <irq.h>


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

#endif // __CONTEXT_H
