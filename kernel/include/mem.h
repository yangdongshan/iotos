#ifndef MEM_H
#define MEM_H

#include <type_def.h>

void mm_init(void *start, size_t size);
void *mm_malloc( size_t size );
void *mm_calloc( size_t num, size_t size );
void *mm_realloc( void *ptr, size_t size );
void  mm_free( void *ptr );
size_t mm_free_size();
#endif // MEM_H
