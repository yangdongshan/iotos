#include <irq.h>
#include <timer.h>
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
    enter_interrupt();
    sys_time_tick();
    timer_tick();
    leave_interrupt();
}
