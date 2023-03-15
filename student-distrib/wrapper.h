#if !defined(WRAPPER_H)
#define WRAPPER_H

#include "exception.h"


#define EXCEPT_LINK(name, func, num) void name();

void DIVISION_ERROR();

EXCEPT_LINK(DIVISION_ERROR, division_error, 0);

#endif
