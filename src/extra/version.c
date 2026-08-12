/* version.c - bcc version banner
 * part of Zythos binutils
 */

#include "syscalls.h"

#define BCC_VERSION     "bcc 1.0.0"
#define BCC_COPYRIGHT   BCC_VERSION " Copyright (C) 2026"

static void my_write_str(const char *s)
{
    int len = 0;
    while (s[len]) len++;
    sys_write(1, s, len);
}

void bcc_print_version(void)
{
    my_write_str(BCC_COPYRIGHT);
    my_write_str("\n");
}