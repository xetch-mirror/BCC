#ifndef BLIBC_ISO646_H
#define BLIBC_ISO646_H

/* Bold C Library - alternative (word-form) operator tokens
 * Same idea as the standard C <iso646.h>: lets you write logical/bitwise
 * operators as words instead of symbols.
 */

#define and    &&
#define or     ||
#define not    !
#define not_eq !=
#define bitand &
#define bitor  |
#define xor    ^
#define compl  ~
#define and_eq &=
#define or_eq  |=
#define xor_eq ^=

/* Digraphs: alternate spellings for braces/brackets, in case your
 * keyboard/toolchain is missing them. */
#define BLIBC_LBRACE <%
#define BLIBC_RBRACE %>
#define BLIBC_LBRACKET <:
#define BLIBC_RBRACKET :>

#endif /* BLIBC_ISO646_H */
