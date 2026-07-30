// stdio.h
#ifndef STDIO_H
#define STDIO_H

#include "stddef.h"

// standard
#define stdin 0
#define stdout 1
#define stderr 2

#define EOF -1

// declaration 
int putchar(int ch);
int puts(char *str);
int printf(char *fmt, ...);

#endif
