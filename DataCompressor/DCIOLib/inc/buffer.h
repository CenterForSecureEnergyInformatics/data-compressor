/* Character buffer (header)
   Part of DataCompressor
   Andreas Unterweger 2013-2015 */

#ifndef _BUFFER_H
#define _BUFFER_H

#include "io.h"

typedef struct buffer_t buffer_t;

typedef io_int_t refill_function_t(void * const buffer_addr, const size_t max_bytes, void * const caller_info);
typedef io_int_t flush_function_t(const void * const buffer_addr, const size_t num_bytes, void * const caller_info);

buffer_t *AllocateBuffer(void);
void FreeBuffer(buffer_t * const buffer);

int InitBuffer(buffer_t * const buffer, const size_t buffer_size);
void UninitBuffer(buffer_t * const buffer);

size_t GetBufferSize(const buffer_t * const buffer);
io_int_t GetUsedBufferSize(const buffer_t * const buffer);

int ResizeBuffer(buffer_t * const buffer, const size_t new_buffer_size);

int RefillBuffer(buffer_t * const buffer, refill_function_t * const refill_func, void * const caller_info);
int FlushBuffer(buffer_t * const buffer, flush_function_t * const flush_func, void * const caller_info);

io_int_t PeekBuffer(const buffer_t * const buffer, uint8_t * const output, const size_t output_size);
io_int_t ReadBuffer(buffer_t * const buffer, uint8_t * const output, const size_t output_size);
io_int_t WriteBuffer(buffer_t * const buffer, const uint8_t * const input, const size_t input_size);

void ClearBuffer(buffer_t * const buffer);

#endif