











#if defined(WIN32)
#if !defined(_INTPTR_T_DEFINED)
typedef INT_PTR intptr_t;
#define _INTPTR_T_DEFINED
#endif
#else
#include <stdint.h>
#endif













