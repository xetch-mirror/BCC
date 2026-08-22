#include "btable.h"

// ---- Lookup functions (declared in btable.h) ----

const OpEntry *btable_find_by_op(OpCode op) {
    for (int i = 0; i < BCONV_OPTABLE_LEN; i++) {
        if (BCONV_OPTABLE[i].op == op) return &BCONV_OPTABLE[i];
    }
    return 0;
}

OpCode btable_find_by_mnemonic(char c0, char c1, int *has_operand) {
    for (int i = 0; i < BCONV_OPTABLE_LEN; i++) {
        if (BCONV_OPTABLE[i].mnemonic[0] == c0 &&
            BCONV_OPTABLE[i].mnemonic[1] == c1) {
            *has_operand = BCONV_OPTABLE[i].has_operand;
            return BCONV_OPTABLE[i].op;
        }
    }
    *has_operand = 0;
    return OP_UNKNOWN;
}

// ---- Code generator ----

#define CODEGEN_MAX 65536

static OpCode g_ops[CODEGEN_MAX];
static int    g_operands[CODEGEN_MAX];
static int    g_count = 0;

void codegen_init(void) {
    g_count = 0;
}

int codegen_emit(OpCode op, int operand) {
    if (g_count >= CODEGEN_MAX) return -1;
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

int codegen_count(void) {
    return g_count;
}

OpCode codegen_op_at(int index) {
    return g_ops[index];
}

int codegen_operand_at(int index) {
    return g_operands[index];
}