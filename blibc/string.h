// string.h
#ifndef STRING_H
#define STRING_H

#include "stddef.h"

int strlen(int *str) {
    int *p;
    p = str;
    while (*p != 0) {
        p = p + 1;
    }
    return p - str;
}

int strcmp(int *s1, int *s2) {
    while (*s1 != 0 && *s1 == *s2) {
        s1 = s1 + 1;
        s2 = s2 + 1;
    }
    return *s1 - *s2;
}

int *strcpy(int *dest, int *src) {
    int *saved;
    saved = dest;
    while (*src != 0) {
        *dest = *src;
        dest = dest + 1;
        src = src + 1;
    }
    *dest = 0;
    return saved;
}

int *memset(int *ptr, int val, int num) {
    int *p;
    p = ptr;
    while (num > 0) {
        *p = val;
        p = p + 1;
        num = num - 1;
    }
    return ptr;
}

#endif
