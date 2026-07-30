// string.h
#ifndef STRING_H
#define STRING_H

#include "stddef.h"

// implementing 
char *strcpy(char *dest, char *src) {
    char *saved = dest;
    while (*src != 0) {
        *dest = *src;
        dest = dest + 1;
        src = src + 1;
    }
    *dest = 0; // term 
    return saved;
}

size_t strlen(char *str) {
    char *p = str;
    while (*p != 0) {
        p = p + 1;
    }
    return p - str;
}

#endif
