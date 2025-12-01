#ifndef PB_TYPES_H_INCLUDED
#define PB_TYPES_H_INCLUDED

#include "pb.h"

#if defined(PB_FIELD_32BIT)
    #define PRIu_pb PRIu32
    #define PRId_pb PRId32
#else
    #define PRIu_pb PRIu16
    #define PRId_pb PRId16
#endif

#endif /* PB_TYPES_H_INCLUDED */
