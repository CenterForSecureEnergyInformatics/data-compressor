/* I/O macros (header only)
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#ifndef _IO_H
#define _IO_H

/* Includes can be found below
   64-bit I/O requires definitions before including stdio.h */

/* Data type size in bits for I/O operations. Sizes other than 32 and 64 have not been tested yet */
#ifndef IO_SIZE_BITS
  #define IO_SIZE_BITS 64
#endif

#if IO_SIZE_BITS != 8 && IO_SIZE_BITS != 16 && IO_SIZE_BITS != 32 && IO_SIZE_BITS != 64
  #error "IO_SIZE_BITS must be either 8, 16, 32 or 64"
#endif

#if IO_SIZE_BITS > 32 /* Potential warnings for this configuration see below (stdio.h needs to be included first) */
  #define _FILE_OFFSET_BITS 64
  #define _LARGEFILE64_SOURCE
  #ifdef _MSC_VER /* MSVC */
    #define FTELL(f) _ftelli64((f))
    #define FSEEK(f, off, orig) _fseeki64((f), (off), (orig))
    #define FOPEN(f, m) fopen((f), (m))
  #elif defined(__GNUC__) /* gcc et al. */
    #define FTELL(f) ftello64((f))
    #define FSEEK(f, off, orig) fseeko64((f), (off), (orig))
    #define FOPEN(f, m) fopen64((f), (m))
  #else /* Assume ftell already uses 64-bit values */
    #define FTELL(f) ftell((f))
    #define FSEEK(f, off, orig) fseek((f), (off), (orig))
    #define FOPEN(f, m) fopen((f), (m))
  #endif
  #define IO_ABS(x) llabs((x))
  #define IO_STRTOUL(str, endptr, base) strtoull((str), (endptr), (base))
#else /* IO_SIZE_BITS <= 32 */
  #define IO_ABS(x) labs(x)
  #define IO_STRTOUL(str, endptr, base) strtoul((str), (endptr), (base))
  #define FTELL(f) ftell((f))
  #define FSEEK(f, off, orig) fseek((f), (off), (orig))
  #define FOPEN(f, m) fopen((f), (m))
#endif

#include <stdio.h>
#include <stdlib.h> /* for llabs et al. */

#include <stdint.h>
#include <limits.h>

#if IO_SIZE_BITS > 32 && ULLONG_MAX < UINT64_MAX
  #error "In order to use 64-bit I/O, unsigned long long needs to be at least 64 bits in size"
#endif

#if CHAR_BIT != 8
  #error "Your machine architecture is not using 8 bits per byte and is therefore not supported"
#endif

#include "common.h"

typedef INT_TYPE(IO_SIZE_BITS) io_int_t;
typedef UINT_TYPE(IO_SIZE_BITS) io_uint_t;

#define MAX_IO_INT_VALUE INT_MAXVALUE(IO_SIZE_BITS)
#define MAX_IO_UINT_VALUE UINT_MAXVALUE(IO_SIZE_BITS)

#if MAX_IO_UINT_VALUE < SIZE_MAX
  #error "io_uint_t must be at least as large as size_t. Increase IO_SIZE_BITS."
#elif MAX_IO_UINT_VALUE == SIZE_MAX
  #define MAX_USABLE_SIZE (SIZE_MAX / 2)
  #define MAX_USABLE_SIZE_BITS (IO_SIZE_BITS - 1)
#else
  #define MAX_USABLE_SIZE SIZE_MAX
  #define MAX_USABLE_SIZE_BITS (8 * sizeof(size_t))
#endif

#define IO_INT_FORMAT INT_FORMAT(IO_SIZE_BITS)
#define IO_UINT_FORMAT UINT_FORMAT(IO_SIZE_BITS)

#ifdef _MSC_VER /* MSVC */
  #define SIZE_T_FORMAT "Iu"
  #define SIZE_T_CAST(x) ((size_t)(x))
#elif defined(__GNUC__) /* gcc et al. */
  #define SIZE_T_FORMAT PRIuMAX
  #define SIZE_T_CAST(x) ((uintmax_t)(x))
#else /* C99 fallback */
  #define SIZE_T_FORMAT "zu"
  #define SIZE_T_CAST(x) ((size_t)(x))
#endif

#define BITSIZE_INFO_PRINT_ARGS "I/O bit size: %" SIZE_T_FORMAT " (usable: %" SIZE_T_FORMAT "), size_t bit size: %" SIZE_T_FORMAT "\n", SIZE_T_CAST(IO_SIZE_BITS), SIZE_T_CAST(MAX_USABLE_SIZE_BITS), SIZE_T_CAST(8 * sizeof(size_t))

#endif