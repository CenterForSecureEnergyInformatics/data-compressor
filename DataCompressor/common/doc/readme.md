Overview
---

common is a collection of headers for other libraries

`log.h`: Provides a printf-style logging macro

`io.h`: Provides types for file I/O as well as ftell and fopen macros (for 64-bit file I/O on platform supports it)

`err_codes.h`: Provides constants for common errors

Notes on usage
---

* `IO_SIZE_BITS` specifies the number of bits used for file-I/O-related operations. In particular, the size of return values for Read/Write functions in dependent libraries are based on it.
* If `IO_SIZE_BITS` is the same size as size_t, the Read/Write functions in dependent libraries do not work properly if the MSB of a size_t variable specifying the size to be read/written is used. For example, if `IO_SIZE_BITS` is 32 and `sizeof(size_t)` is 4, the maximum size (parameter value) that the Read/Write function can work with is `2^31 - 1`, i.e., the 32nd bit cannot be used. If it is used, the return value of the functions will be interpreted as an error (since it is interpreted as a negative number).
* Error codes have to be negative in order to distinguish them from return values which signal the amount of bytes read/written (which is positive).
