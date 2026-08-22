// codegen.h
#ifndef CODEGEN_H
#define CODEGEN_H
#include "bconv_format.h"

#define CODEGEN_MAX 65536

extern OpCode g_ops[CODEGEN_MAX];
extern int    g_operands[CODEGEN_MAX];
extern int    g_count;

void codegen_init(void);
int  codegen_emit(OpCode op, int operand);   // returns index of the emitted instr
int  codegen_here(void);                     // current instruction count
void codegen_patch(int index, int new_operand);

#endif