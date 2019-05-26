
#include <port.h>
#include <typedef.h>

/** init thread initial stack
 *  it looks like the stack keeps registers when it was switched out.
 *
 * FIXME: float point register is not taken into account.
 */
void arch_init_context_frame(struct context_frame *cf,
                             addr_t *entry, void *arg,
                             addr_t *grave)
{
    // hardware saved
    cf->xpsr = 0x01000000;
    // thread start entry
    cf->ip = (uint32_t)entry;
    // if thread return to @no_rt, that's unacceptable
    cf->lr = (uint32_t)grave;
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

