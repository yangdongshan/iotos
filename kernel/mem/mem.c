#include <mem.h>
#include <kdebug.h>
#include <string.h>

#include "umm_malloc.h"

void mem_init(void *start, size_t size)
{
    umm_init(start, size);
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

size_t mm_free_size()
{
    return umm_free_heap_size();
}
