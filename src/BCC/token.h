#ifndef TOKEN_H
#define TOKEN_H

// NOTE: every .c file that includes this header must first do:
//   #include <stdio.h>
//   #include <stdlib.h>
//   ... (whatever system headers it needs)
//   #define int long long
//   #include "token.h"
// c4's trick: redefining `int` as 8 bytes lets pointers be stored
// directly inside `int *` arrays (the code/data "object" arrays).
// It must be defined BEFORE this header on every translation unit,
// or the extern declarations below won't match lex.c's real types.

// tokens and classes (operators last and in precedence order)
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr,
  Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

// opcodes -- this IS bcc's object/VM instruction set, i.e. the format
// your existing .o-reading linker already knows how to consume.
enum { LEA ,IMM ,JMP ,JSR ,BZ  ,BNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PSH ,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,FREE,MSET,MCMP,EXIT };

// value types
enum { CHAR, INT, PTR };

// identifier table row offsets (symbol table is a flat int array,
// Idsz entries per identifier)
enum { Tk, Hash, Name, Class, Type, Val, HClass, HType, HVal, Idsz };

// ---- shared compiler state, defined in lex.c ----
extern char *p, *lp, *data;    // source cursor, line-start cursor, data seg cursor
extern int  *e, *le;           // emitted code cursor, last-printed code cursor
extern int  *id, *sym;         // current identifier, symbol table base
extern int  tk, ival, ty, loc, line; // current token, its value, expr type, local offset, source line
extern int  src, debug;        // flags: print source+asm, print VM trace

// stable base pointers, set once by lex_init(), untouched afterward --
// `le` moves as -s tracing prints, so emit_write_object() needs these
// instead to compute code/data lengths.
extern int  *code_base;
extern char *data_base;

// ---- error handling (defined in handling.c) ----
void error_at(int at_line, const char *fmt, ...); // reports + exits, ties error to a source line
void fatal(const char *fmt, ...);                  // reports + exits, for pre-lexing/CLI failures

// ---- emitter (defined in emit.c) ----
// parse.c never touches the `e` array directly -- it goes through these,
// so emit.c is the only place that knows how instructions get laid out
// and (eventually) how they get written to an .o file on disk.
void  emit(int word);              // append one word (opcode or operand) to the code array
int  *emit_here(void);             // current position in the code array (== c4's `e`)
int  *emit_slot(void);             // reserve+return a patchable operand slot (== c4's `d = ++e`)
void  emit_patch(int *slot, int *target); // backpatch a jump target, cast to int like c4 did
int   emit_last_is(int opcode);    // true if the last emitted word equals opcode (== c4's `*e == X`)
void  emit_undo(void);             // drop the last emitted word (== c4's `--e`)
void  emit_replace_last(int opcode); // overwrite last emitted word in place (== c4's `*e = X`)
int   emit_strcmp_main(char *name); // 0 if name == "main", nonzero otherwise (no libc string.h needed by parse.c)

// Flushes the code/data/symbol pools built by parse_program() to a real
// file on disk in bcc's object format, so it can be handed back into
// bcc's existing .o-reading linker path. Returns 0 on success.
int emit_write_object(const char *out_path, int *idmain);

#endif