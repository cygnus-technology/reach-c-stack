
#ifndef __i3_error_H
#define __i3_error_H

#include <stdio.h>
#include <stdlib.h>
#include "reach-server.h"

// i3_assert() is intended to catch cases unhandled by the programmer. 
// These are programming errors, not recoverable application errors.
// In response to a programming error, stop and show the programmer where they went wrong.

#ifdef BZ6_REACH_PORT
// The BZ6 in a FreeRTOS environment and we have to vTaskDelay in order to let all of 
// the buffered printf's escape.
  #define affirm(a)  if(!(a)) { \
    printf(TEXT_RED "\r\n!!!\r\n!!! affirm() failed in file %s, at %s.%u\r\n!!!\r\n" TEXT_RESET, __FILE__, __FUNCTION__, __LINE__); \
    vTaskDelay(100); \
    __BKPT(0); \
    exit(1); \
    }


  #define i3_assert(a)  if(!(a)) { \
    printf(TEXT_RED "\r\n!!!\r\n!!! i3_assert() failed in file %s, at %s.%u\r\n!!!\r\n" TEXT_RESET, __FILE__, __FUNCTION__, __LINE__); \
    vTaskDelay(100); \
    __BKPT(0); \
    exit(1); \
    }

#else
  #define affirm(a)  if(!(a)) { \
    printf(TEXT_RED "\r\n!!!\r\n!!! affirm() failed in file %s, at %s.%u\r\n!!!\r\n" TEXT_RESET, __FILE__, __FUNCTION__, __LINE__); \
    __asm__("bkpt"); \
    exit(1); \
    }


  #define i3_assert(a)  if(!(a)) { \
    printf(TEXT_RED "\r\n!!!\r\n!!! i3_assert() failed in file %s, at %s.%u\r\n!!!\r\n" TEXT_RESET, __FILE__, __FUNCTION__, __LINE__); \
    __asm__("bkpt"); \
    exit(1); \
    }
#endif

#endif  // ndef __i3_error_H


