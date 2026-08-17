#include <stdio.h>
#include <stdlib.h>
#define int long long
#include "token.h"
#include "lex.h"

void expr(int lev)
{
  int t, *d;

  if (!tk) error_at(line, "unexpected eof in expression");
  else if (tk == Num) { emit(IMM); emit(ival); next(); ty = INT; }
  else if (tk == '"') {
    emit(IMM); emit(ival); next();
    while (tk == '"') next();
    data = (char *)((int)data + sizeof(int) & -sizeof(int)); ty = PTR;
  }
  else if (tk == Sizeof) {
    next(); if (tk == '(') next(); else error_at(line, "open paren expected in sizeof");
    ty = INT; if (tk == Int) next(); else if (tk == Char) { next(); ty = CHAR; }
    while (tk == Mul) { next(); ty = ty + PTR; }
    if (tk == ')') next(); else error_at(line, "close paren expected in sizeof");
    emit(IMM); emit((ty == CHAR) ? sizeof(char) : sizeof(int));
    ty = INT;
  }
  else if (tk == Id) {
    d = id; next();
    if (tk == '(') {
      next();
      t = 0;
      while (tk != ')') { expr(Assign); emit(PSH); ++t; if (tk == ',') next(); }
      next();
      if (d[Class] == Sys) emit(d[Val]);
      else if (d[Class] == Fun) { emit(JSR); emit(d[Val]); }
      else error_at(line, "bad function call");
      if (t) { emit(ADJ); emit(t); }
      ty = d[Type];
    }
    else if (d[Class] == Num) { emit(IMM); emit(d[Val]); ty = INT; }
    else {
      if (d[Class] == Loc) { emit(LEA); emit(loc - d[Val]); }
      else if (d[Class] == Glo) { emit(IMM); emit(d[Val]); }
      else error_at(line, "undefined variable");
      emit(((ty = d[Type]) == CHAR) ? LC : LI);
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
    emit((ty == CHAR) ? LC : LI);
  }
  else if (tk == And) {
    next(); expr(Inc);
    if (emit_last_is(LC) || emit_last_is(LI)) emit_undo(); else error_at(line, "bad address-of");
    ty = ty + PTR;
  }
  else if (tk == '!') { next(); expr(Inc); emit(PSH); emit(IMM); emit(0); emit(EQ); ty = INT; }
  else if (tk == '~') { next(); expr(Inc); emit(PSH); emit(IMM); emit(-1); emit(XOR); ty = INT; }
  else if (tk == Add) { next(); expr(Inc); ty = INT; }
  else if (tk == Sub) {
    next(); emit(IMM);
    if (tk == Num) { emit(-ival); next(); } else { emit(-1); emit(PSH); expr(Inc); emit(MUL); }
    ty = INT;
  }
  else if (tk == Inc || tk == Dec) {
    t = tk; next(); expr(Inc);
    if (emit_last_is(LC)) emit_replace_last(PSH), emit(LC);
    else if (emit_last_is(LI)) emit_replace_last(PSH), emit(LI);
    else error_at(line, "bad lvalue in pre-increment");
    emit(PSH);
    emit(IMM); emit((ty > PTR) ? sizeof(int) : sizeof(char));
    emit((t == Inc) ? ADD : SUB);
    emit((ty == CHAR) ? SC : SI);
  }
  else error_at(line, "bad expression");

  while (tk >= lev) { // precedence climbing
    t = ty;
    if (tk == Assign) {
      next();
      if (emit_last_is(LC) || emit_last_is(LI)) emit_replace_last(PSH);
      else error_at(line, "bad lvalue in assignment");
      expr(Assign); emit(((ty = t) == CHAR) ? SC : SI);
    }
    else if (tk == Cond) {
      next();
      emit(BZ); d = emit_slot();
      expr(Assign);
      if (tk == ':') next(); else error_at(line, "conditional missing colon");
      emit_patch(d, emit_here() + 2); emit(JMP); d = emit_slot();
      expr(Cond);
      emit_patch(d, emit_here());
    }
    else if (tk == Lor) { next(); emit(BNZ); d = emit_slot(); expr(Lan); emit_patch(d, emit_here()); ty = INT; }
    else if (tk == Lan) { next(); emit(BZ);  d = emit_slot(); expr(Or);  emit_patch(d, emit_here()); ty = INT; }
    else if (tk == Or)  { next(); emit(PSH); expr(Xor); emit(OR);  ty = INT; }
    else if (tk == Xor) { next(); emit(PSH); expr(And); emit(XOR); ty = INT; }
    else if (tk == And) { next(); emit(PSH); expr(Eq);  emit(AND); ty = INT; }
    else if (tk == Eq)  { next(); emit(PSH); expr(Lt);  emit(EQ);  ty = INT; }
    else if (tk == Ne)  { next(); emit(PSH); expr(Lt);  emit(NE);  ty = INT; }
    else if (tk == Lt)  { next(); emit(PSH); expr(Shl); emit(LT);  ty = INT; }
    else if (tk == Gt)  { next(); emit(PSH); expr(Shl); emit(GT);  ty = INT; }
    else if (tk == Le)  { next(); emit(PSH); expr(Shl); emit(LE);  ty = INT; }
    else if (tk == Ge)  { next(); emit(PSH); expr(Shl); emit(GE);  ty = INT; }
    else if (tk == Shl) { next(); emit(PSH); expr(Add); emit(SHL); ty = INT; }
    else if (tk == Shr) { next(); emit(PSH); expr(Add); emit(SHR); ty = INT; }
    else if (tk == Add) {
      next(); emit(PSH); expr(Mul);
      if ((ty = t) > PTR) { emit(PSH); emit(IMM); emit(sizeof(int)); emit(MUL); }
      emit(ADD);
    }
    else if (tk == Sub) {
      next(); emit(PSH); expr(Mul);
      if (t > PTR && t == ty) { emit(SUB); emit(PSH); emit(IMM); emit(sizeof(int)); emit(DIV); ty = INT; }
      else if ((ty = t) > PTR) { emit(PSH); emit(IMM); emit(sizeof(int)); emit(MUL); emit(SUB); }
      else emit(SUB);
    }
    else if (tk == Mul) { next(); emit(PSH); expr(Inc); emit(MUL); ty = INT; }
    else if (tk == Div) { next(); emit(PSH); expr(Inc); emit(DIV); ty = INT; }
    else if (tk == Mod) { next(); emit(PSH); expr(Inc); emit(MOD); ty = INT; }
    else if (tk == Inc || tk == Dec) {
      if (emit_last_is(LC)) emit_replace_last(PSH), emit(LC);
      else if (emit_last_is(LI)) emit_replace_last(PSH), emit(LI);
      else error_at(line, "bad lvalue in post-increment");
      emit(PSH); emit(IMM); emit((ty > PTR) ? sizeof(int) : sizeof(char));
      emit((tk == Inc) ? ADD : SUB);
      emit((ty == CHAR) ? SC : SI);
      emit(PSH); emit(IMM); emit((ty > PTR) ? sizeof(int) : sizeof(char));
      emit((tk == Inc) ? SUB : ADD);
      next();
    }
    else if (tk == Brak) {
      next(); emit(PSH); expr(Assign);
      if (tk == ']') next(); else error_at(line, "close bracket expected");
      if (t > PTR) { emit(PSH); emit(IMM); emit(sizeof(int)); emit(MUL); }
      else if (t < PTR) error_at(line, "pointer type expected");
      emit(ADD);
      emit(((ty = t - PTR) == CHAR) ? LC : LI);
    }
    else error_at(line, "compiler error, unexpected token %lld", tk);
  }
}

void stmt(void)
{
  int *a, *b;

  if (tk == If) {
    next();
    if (tk == '(') next(); else error_at(line, "open paren expected");
    expr(Assign);
    if (tk == ')') next(); else error_at(line, "close paren expected");
    emit(BZ); b = emit_slot();
    stmt();
    if (tk == Else) {
      emit_patch(b, emit_here() + 2); emit(JMP); b = emit_slot();
      next();
      stmt();
    }
    emit_patch(b, emit_here());
  }
  else if (tk == While) {
    next();
    a = emit_here() + 1;
    if (tk == '(') next(); else error_at(line, "open paren expected");
    expr(Assign);
    if (tk == ')') next(); else error_at(line, "close paren expected");
    emit(BZ); b = emit_slot();
    stmt();
    emit(JMP); emit((int)a);
    emit_patch(b, emit_here());
  }
  else if (tk == Return) {
    next();
    if (tk != ';') expr(Assign);
    emit(LEV);
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

// Top-level declaration loop -- this is c4's main()'s `while (tk) { ... }`
// block, moved here since it's parsing, not driver logic. Returns the
// symbol-table row for `main`, or NULL if none was defined.
int *parse_program(void)
{
  int bt, ty_, i, *idmain = 0;

  while (tk) {
    bt = INT;
    if (tk == Int) next();
    else if (tk == Char) { next(); bt = CHAR; }
    else if (tk == Enum) {
      next();
      if (tk != '{') next();
      if (tk == '{') {
        next();
        i = 0;
        while (tk != '}') {
          if (tk != Id) error_at(line, "bad enum identifier");
          next();
          if (tk == Assign) {
            next();
            if (tk != Num) error_at(line, "bad enum initializer");
            i = ival;
            next();
          }
          id[Class] = Num; id[Type] = INT; id[Val] = i++;
          if (tk == ',') next();
        }
        next();
      }
    }
    while (tk != ';' && tk != '}') {
      ty_ = bt;
      while (tk == Mul) { next(); ty_ = ty_ + PTR; }
      if (tk != Id) error_at(line, "bad global declaration");
      if (id[Class]) error_at(line, "duplicate global definition");
      next();
      id[Type] = ty_;
      if (!emit_strcmp_main((char *)id[Name])) idmain = id; // track main by name, same intent as c4's idmain
      if (tk == '(') { // function
        id[Class] = Fun;
        id[Val] = (int)emit_here();
        next(); i = 0;
        while (tk != ')') {
          ty_ = INT;
          if (tk == Int) next();
          else if (tk == Char) { next(); ty_ = CHAR; }
          while (tk == Mul) { next(); ty_ = ty_ + PTR; }
          if (tk != Id) error_at(line, "bad parameter declaration");
          if (id[Class] == Loc) error_at(line, "duplicate parameter definition");
          id[HClass] = id[Class]; id[Class] = Loc;
          id[HType]  = id[Type];  id[Type] = ty_;
          id[HVal]   = id[Val];   id[Val] = i++;
          next();
          if (tk == ',') next();
        }
        next();
        if (tk != '{') error_at(line, "bad function definition");
        loc = ++i;
        next();
        while (tk == Int || tk == Char) {
          bt = (tk == Int) ? INT : CHAR;
          next();
          while (tk != ';') {
            ty_ = bt;
            while (tk == Mul) { next(); ty_ = ty_ + PTR; }
            if (tk != Id) error_at(line, "bad local declaration");
            if (id[Class] == Loc) error_at(line, "duplicate local definition");
            id[HClass] = id[Class]; id[Class] = Loc;
            id[HType]  = id[Type];  id[Type] = ty_;
            id[HVal]   = id[Val];   id[Val] = ++i;
            next();
            if (tk == ',') next();
          }
          next();
        }
        emit(ENT); emit(i - loc);
        while (tk != '}') stmt();
        emit(LEV);
        id = sym; // unwind local symbols back to globals
        while (id[Tk]) {
          if (id[Class] == Loc) { id[Class] = id[HClass]; id[Type] = id[HType]; id[Val] = id[HVal]; }
          id = id + Idsz;
        }
      }
      else {
        id[Class] = Glo;
        id[Val] = (int)data;
        data = data + sizeof(int);
      }
      if (tk == ',') next();
    }
    next();
  }
  return idmain;
}