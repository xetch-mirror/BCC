// stdalign.h
#ifndef _STDALIGN_H
#define _STDALIGN.H

// alignment specification mapping to compiler bulitins.

#define alignas _Alignas
#define alignof _Alignof

// mandatory is defined by C STANDARD ISO 

#define __alignas_is_defined
#define __alignof_is_defined

#endif // stdalign.h