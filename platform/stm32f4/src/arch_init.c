#include <arch_debug.h>

int platform_init(void)
{
    arch_debug_init();

    return 0;
}
