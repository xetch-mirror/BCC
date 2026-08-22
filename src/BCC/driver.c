#include <stdio.h>
#include <stdlib.h>
#define int long long
#include "token.h"
#include "lex.h"
#include "btable.h"

void expr(int lev)
{
  int t, *d;

  if (!tk) error_at(line, "unexpected eof in expression");
  else if (tk == Num) { codegen_emit(OP_IMM, ival); next(); ty = INT; }
  else if (tk == '"') {
    codegen_emit(OP_IMM, ival); next();
    while (tk == '"') next();
    data = (char *)((int)data + sizeof(int) & -sizeof(int)); ty = PTR;
  }
  else if (tk == Sizeof) {
    next(); if (tk == '(') next(); else error_at(line, "open paren expected in sizeof");
    ty = INT; if (tk == Int) next(); else if (tk == Char) { next(); ty = CHAR; }
    while (tk == Mul) { next(); ty = ty + PTR; }
    if (tk == ')') next(); else error_at(line, "close paren expected in sizeof");
    codegen_emit(OP_IMM, (ty == CHAR) ? sizeof(char) : sizeof(int));
    ty = INT;
  }
  else if (tk == Id) {
    d = id; next();
    if (tk == '(') {
      next();
      t = 0;
      while (tk != ')') { expr(Assign); codegen_emit(OP_PSH, 0); ++t; if (tk == ',') next(); }
      next();
      if (d[Class] == Sys) codegen_emit(d[Val], 0);
      else if (d[Class] == Fun) codegen_emit(OP_JSR, d[Val]);
      else error_at(line, "bad function call");
      if (t) codegen_emit(OP_ADJ, t);
      ty = d[Type];
    }
    else if (d[Class] == Num) { codegen_emit(OP_IMM, d[Val]); ty = INT; }
    else {
      if (d[Class] == Loc) codegen_emit(OP_LEA, loc - d[Val]);
      else if (d[Class] == Glo) codegen_emit(OP_IMM, d[Val]);
      else error_at(line, "undefined variable");
      codegen_emit(((ty = d[Type]) == CHAR) ? OP_LC : OP_LI, 0);
    }
  }
  else if (tk == '(') {
    next();
    if (tk == Int || tk == Char) {
      t = (tk == Int) ? INT : CHAR; next();
      while (tk == Mul) { next(); t = t + PTR; }
      if (tk == ')') next(); else error_at(line, "bad cast");
      expr(Inc);
      ty = t;
    }
    else {
      expr(Assign);
      if (tk == ')') next(); else error_at(line, "close paren expected");
    }
  }
  else if (tk == Mul) {
    next(); expr(Inc);
    if (ty > INT) ty = ty - PTR; else error_at(line, "bad dereference");
    codegen_emit((ty == CHAR) ? OP_LC : OP_LI, 0);
  }
  else if (tk == And) {
    next(); expr(Inc);
    if (codegen_last_is(OP_LC) || codegen_last_is(OP_LI)) codegen_undo(); else error_at(line, "bad address-of");
    ty = ty + PTR;
  }
  else if (tk == '!') { next(); expr(Inc); codegen_emit(OP_PSH, 0); codegen_emit(OP_IMM, 0); codegen_emit(OP_EQ, 0); ty = INT; }
  else if (tk == '~') { next(); expr(Inc); codegen_emit(OP_PSH, 0); codegen_emit(OP_IMM, -1); codegen_emit(OP_XOR, 0); ty = INT; }
  else if (tk == Add) { next(); expr(Inc); ty = INT; }
  else if (tk == Sub) {
    next();
    if (tk == Num) { codegen_emit(OP_IMM, -ival); next(); }
    else { codegen_emit(OP_IMM, -1); codegen_emit(OP_PSH, 0); expr(Inc); codegen_emit(OP_MUL, 0); }
    ty = INT;
  }
  else if (tk == Inc || tk == Dec) {
    t = tk; next(); expr(Inc);
    if (codegen_last_is(OP_LC)) { codegen_replace_last(OP_PSH); codegen_emit(OP_LC, 0); }
    else if (codegen_last_is(OP_LI)) { codegen_replace_last(OP_PSH); codegen_emit(OP_LI, 0); }
    else error_at(line, "bad lvalue in pre-increment");
    codegen_emit(OP_PSH, 0);
    codegen_emit(OP_IMM, (ty > PTR) ? sizeof(int) : sizeof(char));
    codegen_emit((t == Inc) ? OP_ADD : OP_SUB, 0);
    codegen_emit((ty == CHAR) ? OP_SC : OP_SI, 0);
  }
  else error_at(line, "bad expression");

  while (tk >= lev) {
    t = ty;
    if (tk == Assign) {
      next();
      if (codegen_last_is(OP_LC) || codegen_last_is(OP_LI)) codegen_replace_last(OP_PSH);
      else error_at(line, "bad lvalue in assignment");
      expr(Assign); codegen_emit(((ty = t) == CHAR) ? OP_SC : OP_SI, 0);
    }
    else if (tk == Cond) {
      next();
      d = (int *)(long)codegen_emit(OP_BZ, 0);
      expr(Assign);
      if (tk == ':') next(); else error_at(line, "conditional missing colon");
      codegen_patch((int)(long)d, codegen_here());
      d = (int *)(long)codegen_emit(OP_JMP, 0);
      expr(Cond);
      codegen_patch((int)(long)d, codegen_here());
    }
    else if (tk == Lor) { next(); d = (int *)(long)codegen_emit(OP_BNZ, 0); expr(Lan); codegen_patch((int)(long)d, codegen_here()); ty = INT; }
    else if (tk == Lan) { next(); d = (int *)(long)codegen_emit(OP_BZ, 0);  expr(Or);  codegen_patch((int)(long)d, codegen_here()); ty = INT; }
    else if (tk == Or)  { next(); codegen_emit(OP_PSH, 0); expr(Xor); codegen_emit(OP_OR, 0);  ty = INT; }
    else if (tk == Xor) { next(); codegen_emit(OP_PSH, 0); expr(And); codegen_emit(OP_XOR, 0); ty = INT; }
    else if (tk == And) { next(); codegen_emit(OP_PSH, 0); expr(Eq);  codegen_emit(OP_AND, 0); ty = INT; }
    else if (tk == Eq)  { next(); codegen_emit(OP_PSH, 0); expr(Lt);  codegen_emit(OP_EQ, 0);  ty = INT; }
    else if (tk == Ne)  { next(); codegen_emit(OP_PSH, 0); expr(Lt);  codegen_emit(OP_NE, 0);  ty = INT; }
    else if (tk == Lt)  { next(); codegen_emit(OP_PSH, 0); expr(Shl); codegen_emit(OP_LT, 0);  ty = INT; }
    else if (tk == Gt)  { next(); codegen_emit(OP_PSH, 0); expr(Shl); codegen_emit(OP_GT, 0);  ty = INT; }
    else if (tk == Le)  { next(); codegen_emit(OP_PSH, 0); expr(Shl); codegen_emit(OP_LE, 0);  ty = INT; }
    else if (tk == Ge)  { next(); codegen_emit(OP_PSH, 0); expr(Shl); codegen_emit(OP_GE, 0);  ty = INT; }
    else if (tk == Shl) { next(); codegen_emit(OP_PSH, 0); expr(Add); codegen_emit(OP_SHL, 0); ty = INT; }
    else if (tk == Shr) { next(); codegen_emit(OP_PSH, 0); expr(Add); codegen_emit(OP_SHR, 0); ty = INT; }
    else if (tk == Add) {
      next(); codegen_emit(OP_PSH, 0); expr(Mul);
      if ((ty = t) > PTR) { codegen_emit(OP_PSH, 0); codegen_emit(OP_IMM, sizeof(int)); codegen_emit(OP_MUL, 0); }
      codegen_emit(OP_ADD, 0);
    }
    else if (tk == Sub) {
      next(); codegen_emit(OP_PSH, 0); expr(Mul);
      if (t > PTR && t == ty) { codegen_emit(OP_SUB, 0); codegen_emit(OP_PSH, 0); codegen_emit(OP_IMM, sizeof(int)); codegen_emit(OP_DIV, 0); ty = INT; }
      else if ((ty = t) > PTR) { codegen_emit(OP_PSH, 0); codegen_emit(OP_IMM, sizeof(int)); codegen_emit(OP_MUL, 0); codegen_emit(OP_SUB, 0); }
      else codegen_emit(OP_SUB, 0);
    }
    else if (tk == Mul) { next(); codegen_emit(OP_PSH, 0); expr(Inc); codegen_emit(OP_MUL, 0); ty = INT; }
    else if (tk == Div) { next(); codegen_emit(OP_PSH, 0); expr(Inc); codegen_emit(OP_DIV, 0); ty = INT; }
    else if (tk == Mod) { next(); codegen_emit(OP_PSH, 0); expr(Inc); codegen_emit(OP_MOD, 0); ty = INT; }
    else if (tk == Inc || tk == Dec) {
      if (codegen_last_is(OP_LC)) { codegen_replace_last(OP_PSH); codegen_emit(OP_LC, 0); }
      else if (codegen_last_is(OP_LI)) { codegen_replace_last(OP_PSH); codegen_emit(OP_LI, 0); }
      else error_at(line, "bad lvalue in post-increment");
      codegen_emit(OP_PSH, 0); codegen_emit(OP_IMM, (ty > PTR) ? sizeof(int) : sizeof(char));
      codegen_emit((tk == Inc) ? OP_ADD : OP_SUB, 0);
      codegen_emit((ty == CHAR) ? OP_SC : OP_SI, 0);
      codegen_emit(OP_PSH, 0); codegen_emit(OP_IMM, (ty > PTR) ? sizeof(int) : sizeof(char));
      codegen_emit((tk == Inc) ? OP_SUB : OP_ADD, 0);
      next();
    }
    else if (tk == Brak) {
      next(); codegen_emit(OP_PSH, 0); expr(Assign);
      if (tk == ']') next(); else error_at(line, "close bracket expected");
      if (t > PTR) { codegen_emit(OP_PSH, 0); codegen_emit(OP_IMM, sizeof(int)); codegen_emit(OP_MUL, 0); }
      else if (t < PTR) error_at(line, "pointer type expected");
      codegen_emit(OP_ADD, 0);
      codegen_emit(((ty = t - PTR) == CHAR) ? OP_LC : OP_LI, 0);
    }
    else error_at(line, "compiler error, unexpected token %lld", tk);
  }
}

void stmt(void)
{
  int a, *b;

  if (tk == If) {
    next();
    if (tk == '(') next(); else error_at(line, "open paren expected");
    expr(Assign);
    if (tk == ')') next(); else error_at(line, "close paren expected");
    b = (int *)(long)codegen_emit(OP_BZ, 0);
    stmt();
    if (tk == Else) {
      codegen_patch((int)(long)b, codegen_here());
      b = (int *)(long)codegen_emit(OP_JMP, 0);
      next();
      stmt();
    }
    codegen_patch((int)(long)b, codegen_here());
  }
  else if (tk == While) {
    next();
    a = codegen_here();
    if (tk == '(') next(); else error_at(line, "open paren expected");
    expr(Assign);
    if (tk == ')') next(); else error_at(line, "close paren expected");
    b = (int *)(long)codegen_emit(OP_BZ, 0);
    stmt();
    codegen_emit(OP_JMP, a);
    codegen_patch((int)(long)b, codegen_here());
  }
  else if (tk == Return) {
    next();
    if (tk != ';') expr(Assign);
    codegen_emit(OP_LEV, 0);
    if (tk == ';') next(); else error_at(line, "semicolon expected");
  }
  else if (tk == '{') {
    next();
    while (tk != '}') stmt();
    next();
  }
  else if (tk == ';') {
    next();
  }
  else {
    expr(Assign);
    if (tk == ';') next(); else error_at(line, "semicolon expected");
  }
}