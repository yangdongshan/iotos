#ifndef __STRING_H
#define __STRING_H

#include <type_def.h>


size_t strlen(const char* str);

char* strcpy(char *dst, const char *src);

char* strncpy(char *dst, const char *src, size_t n);

char* strcat(char *dst, const char *src);

int strcmp(const char *src1, const char *src2);

int memcmp(const void *src1, const char *src2, size_t n);

void* memcpy(void *dst, const void *src, size_t n);

void* memset(void *dst, int pat, size_t n);

void* memmove(void *dst, const void *src, size_t n);

#endif // __STRING_H

