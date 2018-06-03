
.equ NVIC_INT_CTRL, 0xE000ED04 // Interrupt control state register.
.equ NVIC_SYSPRI14, 0xE000ED22 // System priority register (priority 14)
.equ NVIC_PENDSV_PRI, 0xFF  //  PendSV priority value (lowest).
.equ NVIC_PENDSVSET,   0x10000000 // Value to trigger PendSV exception.

.thumb_func

/* void arch_context_switch_to(void **sp) */

.global arch_context_switch_to

/* void arch_context_swithc(void **old_sp, void **new_sp) */
.global arch_context_switch


.text
.thumb
.align 2
.syntax unified

    .type arch_context_switch_to, %function
arch_context_switch_to:
    LDR     R2, =NVIC_SYSPRI14
    LDR     R3, =NVIC_PENDSV_PRI
    STRB    R3, [R2]

    MOVS    R2, #0 // Set the PSP to 0 for initial context switch call
    MSR     PSP, R2

    LDR     R2, =NVIC_INT_CTRL // Trigger the PendSV exception (causes context switch)
    LDR     R3, =NVIC_PENDSVSET
    STR     R3, [R2]
    CPSIE   I // Enable interrupts at processor level



    .section .text
    .type arch_context_switch_to, %function
arch_context_switch:



    .type PendSV_Handler, %function
PendSV_Handler:
    CPSID   I // Prevent interruption during context switch
    MRS     R2, PSP  // PSP is process stack pointer
    CBZ     R2, PendSV_Handler_nosave // Skip register save the first time

    SUBS    R2, R2, #0x20 // Save remaining regs r4-11 on process stack
    STM     R2, {R4-R11}
    STR     R2, [R0] // save old thread stack address

PendSV_Handler_nosave:
    LDR     R2, [R1]
    LDR     R2, [R2]
    LDM     R2, {R4-R11}
    ADDS    R2, R2, #0x20
    MSR     PSP, R2
    ORR     LR, LR, #0x04 // Ensure exception return uses process stack
    CPSIE   I
    BX      LR

