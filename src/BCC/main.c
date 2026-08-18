#include <stdio.h>
#include <stdlib.h>
#include "loader.h"
#include "preprocess.h"
#define int long long
#include "token.h"
#include "lex.h"

int *parse_program(void);

#define PREPROCESSED_BUF_SIZE (512 * 1024)

int main(int argc, char **argv)
{
  int *idmain;
  char *raw;
  char *expanded;

  --argc; ++argv;
  if (argc < 2) { fatal("usage: bcc <input.c> <output.o>"); return -1; }

  raw = load_source(argv[0]);
  if (!raw) return -1;

  expanded = malloc(PREPROCESSED_BUF_SIZE);
  if (!expanded) { fatal("could not allocate preprocessing buffer"); return -1; }

  preprocess(raw, expanded, PREPROCESSED_BUF_SIZE);
  free(raw);

  if (lex_init_from_buffer(expanded, 256 * 1024) < 0) return -1;

  idmain = parse_program();
  if (emit_write_object(argv[1], idmain) < 0) return -1;

  printf("wrote %s\n", argv[1]);
  return 0;
}