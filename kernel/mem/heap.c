#include <heap.h>

static heap_t heap;

void heap_init(void)
{
    memset(&heap, 0, sizeof(heap_t));
}

int heap_add_mem_region(void *start, size_t size)
{
    int index = heap.valid_region_count;

    if (index >= MAX_MEM_REGION) {
        KDBG("WARN", "heap mem region full\r\n");
        return -1;
    }

    mem_region_t *region = &heap.region[index++];
    region.start = start;
    region.size = size;

    heap.valid_region_count = index;

    return 0;
}

