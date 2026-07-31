// stdlib.h
#ifndef STDLIB_H
#define STDLIB_H

#include "stddef.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int *malloc(void size_t);
void free(void *ptr);
void exit(int code);
int atoi(const char *str);

#endif
