// string.h
#ifndef STRING_H
#define STRING_H

#include "stddef.h"

size_t strlen(char *str);
int strcmp(char *s1, char *s2);
char *strcpy(char *dest, char *src);
void *memset(void *ptr, int val, size_t num);

#endif
