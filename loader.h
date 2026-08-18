#ifndef LOADER_H
#define LOADER_H

// Reads an entire file into a malloc'd, NUL-terminated buffer.
// Caller owns the returned pointer and must free() it.
// Returns NULL (and prints a message) on any failure.
char *load_source(const char *filename);

#endif