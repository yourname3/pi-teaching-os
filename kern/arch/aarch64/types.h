#ifndef AARCH64_TYPES_H
#define AARCH64_TYPES_H

typedef signed   char   int8_t ;
typedef unsigned char  uint8_t ;
typedef signed   short  int16_t;
typedef unsigned short uint16_t;
typedef signed   int    int32_t;
typedef unsigned int   uint32_t;
typedef signed   long   int64_t;
typedef unsigned long  uint64_t;

typedef unsigned long size_t;
typedef unsigned long uintptr_t;

#define bool  _Bool
#define true  (_Bool)(1)
#define false (_Bool)(0)

#endif