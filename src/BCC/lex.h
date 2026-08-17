#ifndef LEX_H
#define LEX_H

// Allocates the four working pools (symbol table, code, data, source),
// seeds the symbol table with bcc's keywords + builtin syscalls, and
// reads `path` into the source buffer. Returns 0 on success, -1 on
// failure (mirrors c4's original main()-side setup code).
//
// poolsz is applied to sym/code/data/source pools equally (c4 used a
// single 256*1024 constant for all four -- pass that if unsure).
int lex_init(const char *path, int poolsz);

// Advances tk/ival/id (and, as a side effect, interns new identifiers
// into the symbol table) by one token. Same behavior as c4's next().
void next(void);

#endif