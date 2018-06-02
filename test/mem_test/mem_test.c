#include <kdebug.h>
#include <stdio.h>
#include <mem.h>


struct region {
    char *ptr;
    size_t size;
    char pat;
};

struct region mem_region[] = {
    {NULL, 4, 0x04},
    {NULL, 8, 0x08},
    {NULL, 16, 0x16},
    {NULL, 32, 0x32},
    {NULL, 64, 0x64},
    {NULL, 128, 0x28},
    {NULL, 256, 0x56},
    {NULL, 512, 0x12},
};




void mem_test()
{
    int num = sizeof(mem_region)/sizeof(struct region);
    int i;

    for (i = 0; i < num; i++) {
        char *ptr;
        size_t size = mem_region[i].size;
        char pat = mem_region[i].pat;
        ptr = mm_malloc(mem_region[i].size);
        mem_region[i].ptr =  ptr;
        memset(ptr, pat, size);
    }

    mm_free_size();

    for (i = 0; i < num; i++) {
        char *ptr = mem_region[i].ptr;
        size_t size = mem_region[i].size;
        char pat = mem_region[i].pat;
       int j;
       for (j = 0; j < size; j++) {
            if (ptr[j] != pat) {
                kdebug_print("error, expect 0x%x at %p, actual 0%x\r\n",
                        pat, &ptr[j], (char)ptr[j]);

                return;
            }
       }

       mm_free(ptr);
    }

    mm_free_size();
}
