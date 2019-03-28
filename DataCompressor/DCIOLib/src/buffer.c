/* Character buffer
   Part of DataCompressor
   Andreas Unterweger 2013-2015 */

#include "err_codes.h"
#include "buffer.h"

#include <string.h>

struct buffer_t
{
  uint8_t *buffer;
  size_t buffer_size;
  io_int_t buffer_start;
  io_int_t buffer_end;
};

buffer_t *AllocateBuffer(void)
{
  return (buffer_t*)malloc(sizeof(buffer_t));
}

void FreeBuffer(buffer_t * const buffer)
{
  free(buffer);
}

void ClearBuffer(buffer_t * const buffer)
{
  buffer->buffer_start = 0;
  buffer->buffer_end = -1;
}

int InitBuffer(buffer_t * const buffer, const size_t buffer_size)
{
  if (buffer_size <= 0)
    return ERROR_INVALID_VALUE;
  if (buffer_size > MAX_USABLE_SIZE)
    return ERROR_VALUE_LARGER_THAN_USABLE_SIZE;
  buffer->buffer_size = buffer_size;
  if ((buffer->buffer = (uint8_t*)malloc(sizeof(uint8_t) * buffer_size)) == NULL)
    return ERROR_MEMORY;
  ClearBuffer(buffer);
  return NO_ERROR;
}

void UninitBuffer(buffer_t * const buffer)
{
  free(buffer->buffer);
}

size_t GetBufferSize(const buffer_t * const buffer)
{
  return buffer->buffer_size;
}

io_int_t GetUsedBufferSize(const buffer_t * const buffer)
{
  return buffer->buffer_end - buffer->buffer_start + 1;
}

int ResizeBuffer(buffer_t * const buffer, const size_t new_buffer_size)
{
  uint8_t *new_buf;
  io_int_t old_used_size = buffer->buffer_end - buffer->buffer_start + 1;
  if (new_buffer_size <= 0)
    return ERROR_INVALID_VALUE;
  if (new_buffer_size > MAX_USABLE_SIZE)
    return ERROR_VALUE_LARGER_THAN_USABLE_SIZE;
  if ((new_buf = (uint8_t*)malloc(sizeof(uint8_t) * new_buffer_size)) == NULL)
    return ERROR_MEMORY;
  memcpy(new_buf, &buffer->buffer[buffer->buffer_start], (size_t)old_used_size); /* Copy old data */
  free(buffer->buffer);
  buffer->buffer = new_buf;
  buffer->buffer_size = new_buffer_size;
  buffer->buffer_start = 0;
  buffer->buffer_end = old_used_size - 1;
  return NO_ERROR;
}

int RefillBuffer(buffer_t * const buffer, refill_function_t * const refill_func, void * const caller_info)
{
  size_t i;
  const io_int_t old_size = GetUsedBufferSize(buffer);
  for (i = 0; i < (size_t)old_size; i++)
    buffer->buffer[i] = buffer->buffer[buffer->buffer_start + i];
  buffer->buffer_start = 0;
  buffer->buffer_end = old_size - 1;
  if (refill_func != NULL)
  {
    const io_int_t size = (*refill_func)(&buffer->buffer[buffer->buffer_end + 1], (size_t)(buffer->buffer_size - old_size), caller_info);
    if (size > 0)
      buffer->buffer_end += size;
    else
      return (int)size; /* Return signalled error */
  }
  return NO_ERROR;
}

io_int_t PeekBuffer(const buffer_t * const buffer, uint8_t * const output, const size_t output_size)
{
  const io_int_t size = GetUsedBufferSize(buffer) < (io_int_t)output_size ? GetUsedBufferSize(buffer) : (io_int_t)output_size;
  if (output_size > MAX_USABLE_SIZE)
    return ERROR_VALUE_LARGER_THAN_USABLE_SIZE;  
  memcpy(output, &buffer->buffer[buffer->buffer_start], (size_t)size);
  return size;
}

io_int_t ReadBuffer(buffer_t * const buffer, uint8_t * const output, const size_t output_size)
{
  const io_int_t size = PeekBuffer(buffer, output, output_size);
  if (size > 0)
    buffer->buffer_start += size;
  return size;
}

int FlushBuffer(buffer_t * const buffer, flush_function_t * const flush_func, void * const caller_info)
{
  if (flush_func != NULL)
  {
    const io_int_t old_size = GetUsedBufferSize(buffer);
    if (old_size > 0)
    {
      const io_int_t size = (*flush_func)(&buffer->buffer[buffer->buffer_start], (size_t)old_size, caller_info);
      if (size > 0)
        buffer->buffer_start += size;
      return NO_ERROR;
    }
  }
  return NO_ERROR;
}

io_int_t WriteBuffer(buffer_t * const buffer, const uint8_t * const input, const size_t input_size)
{
  const io_int_t old_size = GetUsedBufferSize(buffer);
  const io_int_t free_size = GetBufferSize(buffer) - old_size;
  const size_t size = (size_t)(free_size < (io_int_t)input_size ? free_size : (io_int_t)input_size);
  if (input_size > MAX_USABLE_SIZE)
    return ERROR_VALUE_LARGER_THAN_USABLE_SIZE;
  if (buffer->buffer_end + size >= buffer->buffer_size) /* Make sure that there is no index overflow */
  {
    if (old_size > 0)
      memcpy(&buffer->buffer[0], &buffer[buffer->buffer_start], (size_t)old_size); /* Move elements backwards (this would not be necessary if a ring buffer was used) */
    buffer->buffer_start = 0;
    buffer->buffer_end = old_size - 1;
  }
  memcpy(&buffer->buffer[buffer->buffer_end + 1], input, size);
  buffer->buffer_end += size;
  return size;
}
