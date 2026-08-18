#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define int long long
#include "token.h"
#include "lex.h"

// ---- definitions of the shared state declared extern in token.h ----
char *p, *lp, *data;
int  *e, *le;
int  *id, *sym;
int  tk, ival, ty, loc, line;
int  src, debug;
int  *code_base;
char *data_base;

// shared setup: allocate sym/code/data pools, seed keywords+syscalls.
// Does NOT touch `p`/`lp` (the source cursor) -- callers set those.
static int lex_init_pools(int poolsz)
{
  int i;

  if (!(sym = malloc(poolsz)))       { fatal("could not malloc symbol area"); return -1; }
  if (!(le = e = malloc(poolsz)))    { fatal("could not malloc code area");   return -1; }
  if (!(data = malloc(poolsz)))      { fatal("could not malloc data area");   return -1; }
  code_base = e;
  data_base = data;

  memset(sym,  0, poolsz);
  memset(e,    0, poolsz);
  memset(data, 0, poolsz);

  // seed keywords + builtin syscalls into the symbol table -- next()
  // needs these interned before real lexing starts. Uses a throwaway
  // local string as source; doesn't touch the real `p`/`lp` the caller
  // is about to set up.
  p = "char else enum if int return sizeof while "
      "open read close printf malloc free memset memcmp exit void main";
  i = Char; while (i <= While) { next(); id[Tk] = i++; }
  i = OPEN; while (i <= EXIT)  { next(); id[Class] = Sys; id[Type] = INT; id[Val] = i++; }
  next(); id[Tk] = Char; // reuse Char slot to represent `void`
  next();                // leaves `id` pointed at "main"'s symbol row

  return 0;
}

int lex_init(const char *path, int poolsz)
{
  int fd, i;

  if ((fd = open(path, 0)) < 0) {
    fatal("could not open source file %s", path);
    return -1;
  }

  if (lex_init_pools(poolsz) < 0) return -1;

  if (!(lp = p = malloc(poolsz))) { fatal("could not malloc source area"); return -1; }
  if ((i = read(fd, p, poolsz - 1)) <= 0) { fatal("read() of %s returned %lld", path, i); return -1; }
  p[i] = 0;
  close(fd);

  line = 1;
  next(); // prime the first real token
  return 0;
}

int lex_init_from_buffer(char *source_buf, int poolsz)
{
  if (lex_init_pools(poolsz) < 0) return -1;

  lp = p = source_buf; // caller owns/frees this buffer
  line = 1;
  next(); // prime the first real token
  return 0;
}

void next(void)
{
  char *pp;

  while ((tk = *p)) {
    ++p;
    if (tk == '\n') {
      if (src) {
        printf("%lld: %.*s", line, (int)(p - lp), lp);
        lp = p;
        while (le < e) {
          printf("%8.4s", &"LEA ,IMM ,JMP ,JSR ,BZ  ,BNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PSH ,"
                           "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
                           "OPEN,READ,CLOS,PRTF,MALC,FREE,MSET,MCMP,EXIT,"[*++le * 5]);
          if (*le <= ADJ) printf(" %lld\n", *++le); else printf("\n");
        }
      }
      ++line;
    }
    else if (tk == '#') {
      while (*p != 0 && *p != '\n') ++p;
    }
    else if ((tk >= 'a' && tk <= 'z') || (tk >= 'A' && tk <= 'Z') || tk == '_') {
      pp = p - 1;
      while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_')
        tk = tk * 147 + *p++;
      tk = (tk << 6) + (p - pp);
      id = sym;
      while (id[Tk]) {
        if (tk == id[Hash] && !memcmp((char *)id[Name], pp, p - pp)) { tk = id[Tk]; return; }
        id = id + Idsz;
      }
      id[Name] = (int)pp;
      id[Hash] = tk;
      tk = id[Tk] = Id;
      return;
    }
    else if (tk >= '0' && tk <= '9') {
      if ((ival = tk - '0')) { while (*p >= '0' && *p <= '9') ival = ival * 10 + *p++ - '0'; }
      else if (*p == 'x' || *p == 'X') {
        while ((tk = *++p) && ((tk >= '0' && tk <= '9') || (tk >= 'a' && tk <= 'f') || (tk >= 'A' && tk <= 'F')))
          ival = ival * 16 + (tk & 15) + (tk >= 'A' ? 9 : 0);
      }
      else { while (*p >= '0' && *p <= '7') ival = ival * 8 + *p++ - '0'; }
      tk = Num;
      return;
    }
    else if (tk == '/') {
      if (*p == '/') { ++p; while (*p != 0 && *p != '\n') ++p; }
      else { tk = Div; return; }
    }
    else if (tk == '\'' || tk == '"') {
      pp = data;
      while (*p != 0 && *p != tk) {
        if ((ival = *p++) == '\\') {
          if ((ival = *p++) == 'n') ival = '\n';
        }
        if (tk == '"') *data++ = ival;
      }
      ++p;
      if (tk == '"') ival = (int)pp; else tk = Num;
      return;
    }
    else if (tk == '=') { if (*p == '=') { ++p; tk = Eq; } else tk = Assign; return; }
    else if (tk == '+') { if (*p == '+') { ++p; tk = Inc; } else tk = Add; return; }
    else if (tk == '-') { if (*p == '-') { ++p; tk = Dec; } else tk = Sub; return; }
    else if (tk == '!') { if (*p == '=') { ++p; tk = Ne; } return; }
    else if (tk == '<') { if (*p == '=') { ++p; tk = Le; } else if (*p == '<') { ++p; tk = Shl; } else tk = Lt; return; }
    else if (tk == '>') { if (*p == '=') { ++p; tk = Ge; } else if (*p == '>') { ++p; tk = Shr; } else tk = Gt; return; }
    else if (tk == '|') { if (*p == '|') { ++p; tk = Lor; } else tk = Or; return; }
    else if (tk == '&') { if (*p == '&') { ++p; tk = Lan; } else tk = And; return; }
    else if (tk == '^') { tk = Xor; return; }
    else if (tk == '%') { tk = Mod; return; }
    else if (tk == '*') { tk = Mul; return; }
    else if (tk == '[') { tk = Brak; return; }
    else if (tk == '?') { tk = Cond; return; }
    else if (tk == '~' || tk == ';' || tk == '{' || tk == '}' || tk == '(' || tk == ')' ||
             tk == ']' || tk == ',' || tk == ':') return;
  }
}