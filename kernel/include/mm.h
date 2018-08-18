#ifndef MM_H
#define MM_H

#include <typedef.h>

void mm_init_early(void);

void *mm_malloc( size_t size );
void *mm_calloc( size_t num, size_t size );
void *mm_realloc( void *ptr, size_t size );
void  mm_free( void *ptr );

void *mm_info(void);
size_t mm_free_size();


#define malloc(s) mm_malloc(s)
#define calloc(n, s) mm_calloc(n, s)
#define realloc(p, s) mm_realloc(p, s)
#define free(p) mm_free(p);

#endif // MM_H
