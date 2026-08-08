// stdnoreturn.h
#ifndef noreturn
    #if defined(__GNUC__) || defined(__clang__)
        #define noreturn __attribute__((__noreturn__))
    #else
        #define noreturn _Noreturn
    #endif
    #define __noreturn_is_defined 1
#endif