#include <stdio.h>
#include <stdlib.h>
#include <type_def.h>
#include <kdebug.h>
#include <arch.h>
#include <board.h>
#include <mem.h>
#include <stm32f4xx_conf.h>
#include <testcase.h>

void ms_delay(int ms)
{
    while (ms-- > 0) {
        volatile int x = 5971;
        while (x--) {
            __asm("nop");
        }
    }
}

const extern char _sdata[], _edata[];
const extern char _sbss[], _ebss[];
const extern char _estack[];

#ifdef CONFIG_BOOT_STACK_SIZE
const size_t boot_stack_size = CONFIG_BOOT_STACK_SIZE;
#else
const size_t boot_stack_size = 0x400;
#endif

int os_start()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    arch_init();

    KDBG(INFO, "*** welcome to iotos *** \r\n\r\n");
    KDBG(INFO, "system data section %p size 0x%x\r\n",
            _sdata, _edata - _sdata);
    KDBG(INFO, "system bss section  %p size 0x%x\r\n",
            _sbss, _ebss - _sbss);

    KDBG(INFO, "init board\r\n");
    board_init();

    size_t size = _estack - _ebss - boot_stack_size;
    KDBG(INFO, "init mem, start at %p, size %dKB\r\n",
            _ebss, size >> 10);
    mem_init(_ebss, size);
    void *ptr = mm_malloc(128);
    KDBG(INFO, "malloc at %p\r\n", ptr);

    mem_test();

    KDBG(INFO, "os_init\r\n");
    //os_init();

    //user_main();

    //thread_become_idle();

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;

    GPIO_Init(GPIOD, &GPIO_InitStructure);

    for ( ; ;)  {
        ms_delay(500);
        GPIO_SetBits(GPIOD, GPIO_Pin_12);
        GPIO_ResetBits(GPIOD, GPIO_Pin_13);
        ms_delay(500);
        GPIO_ResetBits(GPIOD, GPIO_Pin_12);
        GPIO_SetBits(GPIOD, GPIO_Pin_13);
    }

    return 0;
}
