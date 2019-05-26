
.syntax unified
.thumb
.file "ports.S"
.text

.global arch_start_first_task
.global arch_context_switch
.global PendSV_Handler

.extern g_new_task
.extern g_cur_task

.type arch_start_first_task, function
arch_start_first_task:
    cpsid   i
    movs    r2, #0
    msr     psp, r2
    ldr     r2, =0xE000ED22 // set pendsv priority to lowest priority
    ldr     r3, =0xff
    str     r3, [r2]
    ldr     r2, =0xE000ED04 // trigger pendsv exception
    ldr     r3, =0x10000000
    str     r3, [r2]
    cpsie   i
    bx      lr

.type arch_context_switch, function
arch_context_switch:
    cpsid   i
    ldr     r0, =0xE000ED04
    ldr     r1, =0x10000000
    str     r1, [r0]
    cpsie   i
    bx     lr

.type PendSV_Handler, function
PendSV_Handler:
    cpsid   i
    mrs     r1, psp
    cbz     r1, 1f
    stmdb   r1!, {r4-r11}
    ldr     r0, =g_cur_task
    ldr     r0, [r0]
    str     r1, [r0]
1:
    ldr     r0, =g_new_task
    ldr     r0, [r0]
    ldr     r1, =g_cur_task
    str     r0, [r1]
    ldr     r1, [r0]
    ldmia   r1!, {r4-r11}
    msr     psp, r1
    orr     lr, lr, #0x04
    cpsie   i
    bx     lr


