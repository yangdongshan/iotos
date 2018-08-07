
#include <port.h>
#include <typedef.h>

void __attribute__((optimize("O2"))) arch_context_switch_to(unsigned char* sp) 
{
    __asm__ (
             "cpsid i\r\n"
             "movs  r2, #0\t\n"
             "msr   psp, r2\t\n"
             "ldr r2, =0xE000ED22\t\n" // set pendsv priority to lowest priority
             "ldr r3, =0xff\t\n"
             "str r3, [r2]\t\n"
             "ldr r2, =0xE000ED04\t\n" // trigger pendsv exception
             "ldr r3, =0x10000000\t\n"
             "str r3, [r2]\t\n"
             "cpsie i"
             );
}

void __attribute__((optimize("O2"))) arch_context_switch(unsigned char *new_sp, unsigned char *old_sp) 
{
    __asm__ (
             "cpsid i\t\n"
             "ldr r2, =0xE000ED04\t\n"
             "ldr r3, =0x10000000\t\n"
             "str r3, [r2]\t\n"
             "cpsie i\t\n"
            );
}


/**
 *  r0: new thread sp addr
 *  r1: old thread sp addr
 */
void __attribute__((optimize("O2"))) PendSV_Handler(void)
{
    __asm__ (
             "cpsid i\t\n"
             "mrs    r2, psp\t\n"
             "cbz    r2, 1f\t\n"
             "subs r2, r2, #0x20\t\n"
             "stm r2, {r4-r11}\t\n"
             "str r2, [r1]\t\n"
             "1:\t\n"
             "ldr r2, [r0]\t\n"
             "ldm r2, {r4-r11}\t\n"
             "adds r2, r2, #0x20\t\n"
             "msr psp, r2\t\n"
             "orr lr, lr, #0x04\t\n"
             "cpsie i"
            );
}

void no_ret_func(void)
{

}

/** init thread initial stack
 *  it looks like the stack keeps registers when it was switched out.
 *
 * FIXME: float point register is not taken into account.
 */
void arch_init_context_frame(struct context_frame *cf, addr_t *entry, void *arg)
{
    // hardware saved
    cf->xpsr = 0x01000000;
    // thread start entry
    cf->ip = (uint32_t)entry;
    // if thread return to @no_rt, that's unacceptable
    cf->lr = (uint32_t)no_ret_func;
    cf->r12 = 0x12121212;
    cf->r3 =  0x03030303;
    cf->r2 =  0x02020202;
    cf->r1 =  0x01010101;
    cf->r0 = (uint32_t)arg;

    // software saved
    cf->r11 = 0x11111111;
    cf->r10 = 0x10101010;
    cf->r9 =  0x09090909;
    cf->r8 =  0x08080808;
    cf->r7 =  0x07070707;
    cf->r6 =  0x06060606;
    cf->r5 =  0x05050505;
    cf->r4 =  0x04040404;
}

