#include <stdio.h>
#include <stdlib.h>
#define int long long
#include "token.h"

// e/le/data/sym are defined in lex.c; emit.c is the only file besides
// lex.c that's allowed to poke the raw arrays directly. parse.c goes
// through the functions below instead.

void emit(int word) { *++e = word; }

int *emit_here(void) { return e; }

int *emit_slot(void) { return ++e; } // reserve a word now, patch it later

void emit_patch(int *slot, int *target) { *slot = (int)target; }

int emit_last_is(int opcode) { return *e == opcode; }

void emit_undo(void) { --e; }

void emit_replace_last(int opcode) { *e = opcode; }

int emit_strcmp_main(char *name)
{
  const char *m = "main";
  while (*m) { if (*name != *m) return 1; ++name; ++m; }
  return *name != 0;
}

// ---- object file writer ----
//
// This is new -- c4 never wrote object files, it interpreted straight
// out of the `e` array. Format here is intentionally simple; swap the
// header/section layout below for whatever bcc's existing linker
// already expects if it's not this.
//
//   magic   "BCC1"      4 bytes
//   entry   int         offset (in words) of main(), or -1 if none
//   codelen int         number of words in the code section
//   datalen int          bytes in the data section
//   code[codelen]        int words, verbatim from the e[] array
//   data[datalen]        raw bytes, verbatim from the data[] array
//
int emit_write_object(const char *out_path, int *idmain)
{
  FILE *f;
  int entry, codelen, datalen;

  f = fopen(out_path, "wb");
  if (!f) { fatal("could not open %s for writing", out_path); return -1; }

  entry   = idmain ? idmain[Val] - (int)code_base : -1; // store entry as an offset, not a raw pointer
  codelen = e - code_base;          // words emitted since lex_init allocated the pool
  datalen = data - data_base;       // bytes written into the data segment (strings etc.)

  fwrite("BCC1", 1, 4, f);
  fwrite(&entry, sizeof(int), 1, f);
  fwrite(&codelen, sizeof(int), 1, f);
  fwrite(&datalen, sizeof(int), 1, f);
  fwrite(code_base, sizeof(int), codelen, f);
  fwrite(data_base, 1, datalen, f);

  fclose(f);
  return 0;
}