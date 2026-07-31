# BCC — C in Four Functions

A minimalist, self-hosting C compiler. BCC is a fork of [c4](https://github.com/rswier/c4) by Robert Swier, extended with a preprocessor, a loader, and a small bare-metal-oriented C library (`blibc`).

## What is this

The original c4 compiles a small but real subset of C — just enough to compile itself — in about four functions: `next` (lexer), `expr` (expression parser), `stmt` (statement parser), and `main`. BCC builds on that same idea while adding a few extra pieces for practical use.

## Files

| File | Purpose |
|---|---|
| `c4.c` | The core compiler — lexer, parser, and virtual machine in one file. |
| `hello.c` | A minimal demo program you can compile and run with BCC. |
| `loader.c` | <!-- TODO: confirm — loads/executes compiled bytecode or object output? --> |
| `preprocessor.c` | <!-- TODO: confirm — handles `#include`/`#define` before the main compile pass? --> |
| `blibc/` | <!-- TODO: confirm — minimal C standard library implementation used when targeting bare metal? --> |
| `Makefile` | Build script for the compiler and its components. |

## Building

```
make
```

Or manually with gcc:

```
gcc -o c4 c4.c
```

## Usage

Compile and run a C source file directly:

```
./c4 hello.c
```

Run in step-trace mode:

```
./c4 -s hello.c
```

Compile the compiler with itself, then run a program:

```
./c4 c4.c hello.c
```

Compile the compiler with itself compiling itself, then run a program:

```
./c4 c4.c c4.c hello.c
```

## Supported C subset

Like the original c4, BCC only supports a small subset of C: `char`, `int`, and pointer types; `if`, `while`, `return`, and basic expressions; enough to be self-hosting. It is not a general-purpose, standards-compliant C compiler — it's built for minimalism and clarity.

## Credits

Built on [c4](https://github.com/rswier/c4) by Robert Swier — see `Thanks Robert.md`.

## License

See [LICENSE](./LICENSE).
