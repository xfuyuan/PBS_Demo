#include <stdio.h>

void serial_putc(char chr)
{
    putchar(chr);
}

char serial_getc(void)
{
    return getchar();
}