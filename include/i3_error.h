
#ifndef __i3_error_H
#define __i3_error_H

#include <stdio.h>
#include <stdlib.h>
#include "reach-server.h"

// i3_assert() is intended to catch cases unhandled by the programmer. 
// These are programming errors, not recoverable application errors.
// In response to a programming error, stop and show the programmer where they went wrong.

extern void i3_log_fatal_error(char * msg);

#ifdef DEV_BUILD
  extern char gFatalErrorBuffer[256];

  #define i3_assert(a)  if(!(a)) { \
    sprintf(gFatalErrorBuffer, TEXT_RED "\r\n!!!\r\n!!! i3_assert() failed in file %s, at %s.%u\r\n!!!\r\n" TEXT_RESET, __FILE__, __FUNCTION__, __LINE__); \
    i3_log_fatal_error(gFatalErrorBuffer); \
  }

    #define affirm(a)  if(!(a)) { \
    sprintf(gFatalErrorBuffer, TEXT_RED "\r\n!!!\r\n!!! affirm() failed in file %s, at %s.%u\r\n!!!\r\n" TEXT_RESET, __FILE__, __FUNCTION__, __LINE__); \
    i3_log_fatal_error(gFatalErrorBuffer); \
  }

#else

  #define i3_assert(a)  if(!(a)) { \
    i3_log_fatal_error((NULL); \
  }


  #define affirm(a)  if(!(a)) { \
    i3_log_fatal_error((NULL); \
  }

#endif // def DEV_BUILD



#endif  // ndef __i3_error_H


