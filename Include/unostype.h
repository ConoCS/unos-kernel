#ifndef UNOS_TYPES_H
#define UNOS_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <limits.h>

// another define
#if defined(__x86_64__) || defined(_M_X64)
#define ARCH_X64
#endif

#ifdef __cplusplus
  #undef NULL
  #define NULL 0
#else
  #undef NULL
  #define NULL ((void*)0)
#endif

#define TRUE 1
#define FALSE 0
#define TRUE_LEVEL(n) ((n) > 0 ? TRUE : FALSE)

#define STATUS_OK       1
#define STATUS_FAIL     0
#define STATUS_BUSY    -1
#define STATUS_TIMEOUT -2
#define STATUS_UNKNOWN -3
#define STATUS_INVALID -4

#ifndef SINLINE
#define SINLINE static inline
#endif

#ifndef INLINE
#define INLINE inline
#endif

#define STATIC static
#define GLOBAL extern

#define VUNUSED

// This is value and variable definition custom for UnOS
// Basic redefinitions
#define IN
#define OUT
#define OPTIONAL
#define CONST       const
#define CONST_IN    const
typedef const void* CONST_IN_PTR;

// Basic types


typedef void     VOID;
typedef int      INT;
typedef unsigned int UINT;
typedef float    FLOT;
typedef bool     BOOL;
#define BOOLFALSE false
#define BOOLTRUE true

// Char types
typedef char     CHARA8;
typedef wchar_t  CHARA16;

// Unsigned Ints
typedef uint8_t  USINT8;
typedef uint16_t USINT16;
typedef uint32_t USINT32;
typedef uint64_t USINT64;

// Signed Ints
typedef int8_t   INT8;
typedef int16_t  INT16;
typedef int32_t  INT32;
typedef int64_t  INT64;

// Virtual / Physical
typedef void*    VPTR;
typedef size_t   VSIZE;

// Misc
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef uint8_t  BYTE;
typedef uint16_t WORD;

typedef uintptr_t UINTPTR;
typedef intptr_t  INTPTR;
typedef UINTPTR   VPTR_UINTPTR; // Virtual pointer as uintptr_t
typedef INTPTR   VPTR_INTPTR; // Virtual pointer as intptr_t


#ifdef ARCH_X64
#define UINTN uint64_t
#define INTN   int64_t
#else
#define UINTN  uint32_t
#define INTN   int32_t
#endif

#define UNUSED(x) (void)(x)

#ifndef _VALUE_RETURN_TYPE_
#define _VALUe_RETURN_TYPE_

typedef void FUNCNOSTATUS;
typedef int FUNCWITHSTATUS;

#endif

#endif
