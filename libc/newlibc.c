#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>


#undef errno
extern int errno;
extern int  _end;

void *_sbrk ( int incr )
{
	return NULL;
}

int link(char *old, char *new)
{
	return -1;
}

int _close(int file)
{
  return -1;
}

int _fstat(int file, struct stat *st)
{
	return 0;
}

int _isatty(int file)
{
  return 1;
}

int _lseek(int file, int ptr, int dir)
{
  return 0;
}

int _read(int file, char *ptr, int len)
{
  return 0;
}

void abort(void)
{
  /* Abort called */
  while(1);
}

extern int arch_debug_print(const char *str, int len);
int _write(int file, char *ptr, int len)
{
    arch_debug_print(ptr, len);
    return len;
}

