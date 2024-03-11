
#ifndef __i3_error_H
#define __i3_error_H

#include <stdio.h>
#include <stdlib.h>


#define affirm(a)  if(!(a)) { \
    printf(TEXT_RED "\naffirm() failed at %s.%u\n" TEXT_RESET, __FUNCTION__, __LINE__); \
    __asm__("bkpt"); \
    exit(1); \
    }


#endif  // ndef __i3_error_H


