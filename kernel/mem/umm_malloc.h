#ifndef _UMM_MALLOC_H
#define _UMM_MALLOC_H

#include <type_def.h>

void umm_init(void *start, size_t size);
void *umm_malloc( size_t size );
void *umm_calloc( size_t num, size_t size );
void *umm_realloc( void *ptr, size_t size );
void  umm_free( void *ptr );
size_t umm_free_heap_size(void);

#endif // _UMM_MALLOC_H
