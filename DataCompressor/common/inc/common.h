/* Common macros (header only)
   Part of DataCompressor
   Andreas Unterweger, 2015 */

#ifndef _COMMON_H
#define _COMMON_H

#include <inttypes.h>

#define GLUE(x, y, z) x##y##z
#define GLUE_TYPE(prefix, size) GLUE(prefix, size, _t)
#define GLUE_FORMAT(prefix, size) GLUE(PRI, prefix, size)
#define GLUE_MAX(prefix, size) GLUE(prefix, size, _MAX)

#define INT_TYPE(size) GLUE_TYPE(int, size)
#define INT_FORMAT(size) GLUE_FORMAT(d, size)
#define INT_MAXVALUE(size) GLUE_MAX(INT, size)

#define UINT_TYPE(size) GLUE_TYPE(uint, size)
#define UINT_FORMAT(size) GLUE_FORMAT(u, size)
#define UINT_MAXVALUE(size) GLUE_MAX(UINT, size)

#endif