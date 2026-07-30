// stdio.h
#ifndef STDIO_H
#define STDIO_H

#include "stddef.h"

#define stdin 0
#define stdout 1
#define stderr 2
#define EOF -1

int putchar(int ch);
int puts(int *str);
int printf(int *fmt);

#endif
