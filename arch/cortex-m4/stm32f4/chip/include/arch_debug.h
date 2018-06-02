#ifndef ARCH_DEBUG_H
#define ARCH_DEBUG_H

/*#define arch_putchar putchar
#define arch_getchar getchar
#define arch_puts puts
#define arch_gets gets
*/
void arch_debug_init(void);

int arch_debug_print(const char *str, int len);

#endif // ARCH_DEBUG_H

