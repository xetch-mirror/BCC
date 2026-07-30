// stdlib.h
#ifndef STDLIB_H
#define STDLIB_H

#include "stddef.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int *malloc(int size);
void free(int *ptr);
void exit(int code);
int atoi(int *str);

#endif
