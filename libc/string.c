#include <string.h>

size_t strlen(const char *src)
{
    size_t n = 0;

    while (*src++ != '\0')
        n++;

    return n;
}

char* strcpy(char *dst, const char *src)
{
    char *p = dst;

    while (*p != '\0') {
        *p++ = *src++;
    }

    return dst;
}

char* strncpy(char *dst, const char *src, size_t n)
{
    char *p = dst;

    while (*src != '\0' && n--) {
        *p++ = *src++;
    }

    return dst;
}

char* strcat(char *dst, const char *src)
{
    char *p = dst;

    p += strlen(p);
    while (*src != '\0') {
        *p++ = *src++;
    }
    *p = '\0';

    return dst;
}

int strcmp(const char *src1, const char *src2)
{
    int ret;

    while (1) {
        ret = *src1 - *src2;
        if (!*src1 || !ret)
            break;

        src1++;
        src2++;
    }

    return ret;
}

int memcmp(const void *src1, const char *src2, size_t n)
{
    uint8_t *p1, *p2;

    p1 = (uint8_t*)src1;
    p2 = (uint8_t*)src2;

    while (n-- > 0) {
        if (*p1 > *p2) {
            return 1;
        } else if (*p1 < *p2) {
            return -1;
        } else {
            p1++;
            p2++;
        }
    }

    return 0;
}

void* memcpy(void *dst, const void *src, size_t n)
{
    uint8_t *d, *s;

    d = (uint8_t*)dst;
    s = (uint8_t*)src;

    while (n-- > 0) {
        *d++ = *s++;
    }

    return dst;
}

void *memset(void *dst, int pat, size_t n)
{
    uint8_t *p = (uint8_t*)dst;
    uint8_t c = (uint8_t)pat;

    while (n-- > 0) {
        *p++ = c;
    }

    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    uint8_t *d = dst;
    const uint8_t *s = src;

    while (n--) {
        *d++ = *s++;
    }

    return dst;
}
