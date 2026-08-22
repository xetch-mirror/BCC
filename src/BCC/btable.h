#ifndef BTABLE_H
#define BTABLE_H

#define BCONV_MAGIC0 'B'
#define BCONV_MAGIC1 'L'

typedef enum {
    OP_LEA = 0,  OP_IMM,  OP_JMP,  OP_JSR,  OP_BZ,   OP_BNZ,
    OP_ENT,      OP_ADJ,  OP_LEV,  OP_LI,   OP_LC,   OP_SI,
    OP_SC,       OP_PSH,  OP_OR,   OP_XOR,  OP_AND,  OP_EQ,
    OP_NE,       OP_LT,   OP_GT,   OP_LE,   OP_GE,   OP_SHL,
    OP_SHR,      OP_ADD,  OP_SUB,  OP_MUL,  OP_DIV,  OP_MOD,
    OP_OPEN,     OP_READ, OP_CLOS, OP_PRTF, OP_MALC, OP_FREE,
    OP_MSET,     OP_MCMP, OP_EXIT, OP_NOP,  OP_HLT,
    OP_UNKNOWN = -1
} OpCode;

typedef struct {
    const char mnemonic[3];
    OpCode     op;
    int        has_operand;
} OpEntry;

static const OpEntry BCONV_OPTABLE[] = {
    { "L1", OP_LEA,  1 },
    { "K4", OP_IMM,  1 },
    { "BB", OP_JMP,  1 },
    { "C!", OP_JSR,  1 },
    { "Z0", OP_BZ,   1 },
    { "Z1", OP_BNZ,  1 },
    { "E9", OP_ENT,  1 },
    { "A3", OP_ADJ,  1 },
    { "R5", OP_LEV,  0 },
    { "I2", OP_LI,   0 },
    { "I3", OP_LC,   0 },
    { "S2", OP_SI,   0 },
    { "S3", OP_SC,   0 },
    { "P9", OP_PSH,  0 },
    { "O1", OP_OR,   0 },
    { "X1", OP_XOR,  0 },
    { "N2", OP_AND,  0 },
    { "Q0", OP_EQ,   0 },
    { "Q1", OP_NE,   0 },
    { "T0", OP_LT,   0 },
    { "T1", OP_GT,   0 },
    { "T2", OP_LE,   0 },
    { "T3", OP_GE,   0 },
    { "H0", OP_SHL,  0 },
    { "H1", OP_SHR,  0 },
    { "A4", OP_ADD,  0 },
    { "S0", OP_SUB,  0 },
    { "M1", OP_MUL,  0 },
    { "D1", OP_DIV,  0 },
    { "M2", OP_MOD,  0 },
    { "F1", OP_OPEN, 0 },
    { "F2", OP_READ, 0 },
    { "F3", OP_CLOS, 0 },
    { "F4", OP_PRTF, 0 },
    { "M3", OP_MALC, 0 },
    { "M4", OP_FREE, 0 },
    { "M5", OP_MSET, 0 },
    { "M6", OP_MCMP, 0 },
    { "R9", OP_EXIT, 0 },
    { "N0", OP_NOP,  0 },
    { "N9", OP_HLT,  0 },
};

#define BCONV_OPTABLE_LEN (sizeof(BCONV_OPTABLE) / sizeof(OpEntry))

#endif