#ifndef HEAP_H
#define HEAP_H

#include <types.h>

#ifdef CONFIG_MAX_MEM_REGION
#define MAX_MEM_REGION CONFIG_MAX_MEM_REGION
#else
#define MAX_MEM_REGION (3)
#endif

typedef struct {
    void *start;
    unsigned int size;
} mem_region_t;

typedef struct {
    mem_region_t region[MAX_MEM_REGION];
    unsigned int valid_region_count;
} heap_t;

void heap_init(void);

int heap_add_mem_region(void *start, size_t size);

#endif // HEAP_H
