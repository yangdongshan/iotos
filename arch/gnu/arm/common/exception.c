#include <kdebug.h>
#include <port.h>

static dump_exception_frame(struct context_frame *cf)
{

    kdebug_print("=== dump registers ===\r\n");
    kdebug_print(" r0 0x%08x r1 0x%08x  r2 0x%08x  r3 0x%08x\r\n",
                cf->r0, cf->r1, cf->r2, cf->r3);
    kdebug_print(" r4 0x%08x r5 0x%08x  r6 0x%08x  r7 0x%08x\r\n",
                cf->r0, cf->r1, cf->r2, cf->r3);
    kdebug_print(" r8 0x%08x r9 0x%08x r10 0x%08x r11 0x%08x\r\n",
                cf->r0, cf->r1, cf->r2, cf->r3);
    kdebug_print("r12 0x%08x sp 0x%08x  lr 0x%08x  pc 0x%08x\r\n",
                cf->r0, cf->r1, cf->r2, cf->r3);
    kdebug_print("xpsr 0x%08x\r\n",
                cf->xpsr);
}


void HardFault_Handler(void)
{
    __asm__ (
        "push {r4-r11}\t\n"
        "mov r0, sp\t\n"
        "bl dump_exception_frame\t\n"
        );

    while(1);
}

void MemManage_Handler(void)
{
    __asm__ (
        "push {r4-r11}\t\n"
        "mov r0, sp\t\n"
        "bl dump_exception_frame\t\n"
        );

    while(1);
}

void BusFault_Handler(void)
{
    __asm__ (
        "push {r4-r11}\t\n"
        "mov r0, sp\t\n"
        "bl dump_exception_frame\t\n"
        );

    while(1);

}

void UsageFault_Handler(void)
{
    __asm__ (
        "push {r4-r11}\t\n"
        "mov r0, sp\t\n"
        "bl dump_exception_frame\t\n"
        );

    while(1);
}
