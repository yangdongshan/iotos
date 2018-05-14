#include <stm32f4xx_conf.h>

int os_start()
{
    //arch_init();

    //board_init();

    //os_init();

    //user_main();

    //thread_become_idle();

    GPIO_DeInit(GPIOA);
    for ( ; ;)  {

    }

    return 0;
}
