#include <mm.h>
#include <kdebug.h>
#include <string.h>

#include "umm_malloc.h"
#include "umm_malloc_cfg.h"

const extern char _sdata[], _edata[];
const extern char _sbss[], _ebss[];
const extern char _estack[];

#ifdef CONFIG_BOOT_STACK_SIZE
const size_t msp_stack_size = CONFIG_BOOT_STACK_SIZE;
#else
const size_t msp_stack_size = 0x400;
#endif

void mm_init(void)
{
    size_t size = _estack - _ebss - msp_stack_size;

    umm_init(_ebss, size);
}

void *mm_malloc(size_t size)
{
    return umm_malloc(size);
}

void *mm_calloc(size_t num, size_t size)
{
    return umm_calloc(num, size);
}

void *mm_realloc(void *ptr, size_t size)
{
    return umm_realloc(ptr, size);
}

void  mm_free(void *ptr)
{
    umm_free(ptr);
}

void *mm_info(void)
{
    return umm_info(NULL);
}

size_t mm_free_size()
{
    return umm_free_heap_size();
}
