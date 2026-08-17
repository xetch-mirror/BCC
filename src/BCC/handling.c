#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#define int long long
#include "token.h"

// All of c4's `printf("%d: msg\n", line); exit(-1);` call sites become
// one call to error_at() -- same fail-fast behavior (no error recovery
// yet), just centralized so parse.c/lex.c stay readable and it's one
// place to add resync/recovery later if you want it.
void error_at(int at_line, const char *fmt, ...)
{
  va_list ap;
  fprintf(stderr, "%lld: ", at_line);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  exit(-1);
}

// For failures that aren't tied to a source line (bad CLI args, malloc
// failure, file I/O before lexing has a line counter yet).
void fatal(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  exit(-1);
}