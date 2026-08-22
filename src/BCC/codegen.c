// codegen.c
#include "codegen.h"

OpCode g_ops[CODEGEN_MAX];
int    g_operands[CODEGEN_MAX];
int    g_count = 0;

void codegen_init(void) {
    g_count = 0;
}

int codegen_emit(OpCode op, int operand) {
    if (g_count >= CODEGEN_MAX) return -1; // out of space
    g_ops[g_count] = op;
    g_operands[g_count] = operand;
    return g_count++;
}

int codegen_here(void) {
    return g_count;
}

void codegen_patch(int index, int new_operand) {
    g_operands[index] = new_operand;
}