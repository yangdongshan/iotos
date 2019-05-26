#include <irq.h>
#include <tick.h>
#include <time.h>

#include "stm32f4xx_rcc.h"

void systick_start(void)
{
    RCC_ClocksTypeDef rcc_clock;

    RCC_GetClocksFreq(&rcc_clock);
    SysTick_Config(rcc_clock.HCLK_Frequency/CONFIG_TICKS_PER_SEC);
}

void SysTick_Handler(void)
{
    interrupt_enter();
    sys_tick_inc();
    tick_update();
    interrupt_leave();
}
