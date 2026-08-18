#include <stdio.h>
#include <stdlib.h>
#include "loader.h"

char *load_source(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("BCC:could not open file, terminating interpret %s\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(f);
        printf("BCC:can't allocate memory to file, terminated interpret\n");
        return NULL;
    }

    fread(buf, 1, size, f);
    buf[size] = 0;

    fclose(f);
    return buf;
}