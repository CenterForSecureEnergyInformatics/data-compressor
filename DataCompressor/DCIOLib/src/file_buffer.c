/* File I/O buffer
   Part of DataCompressor
   Andreas Unterweger 2013-2015 */

#include "err_codes.h"
#include "file_buffer.h"

#include <stdlib.h>

typedef enum file_buffer_type_t
{
  FBT_FILE = 0,
  FBT_MEMORY = 1
} file_buffer_type_t;

struct file_buffer_t
{
  FILE *base_file;
  buffer_t *io_buffer;
  file_buffer_mode_t mode;
  file_buffer_type_t type;
};

file_buffer_t *AllocateFileBuffer(void)
{
  return (file_buffer_t*)malloc(sizeof(file_buffer_t));
}

void FreeFileBuffer(file_buffer_t * const file_buffer)
{
  free(file_buffer);
}

static io_int_t RefillFromFile(void * const buffer_addr, const size_t max_bytes, void * const caller_info)
{
  if (max_bytes > MAX_USABLE_SIZE)
    return ERROR_VALUE_LARGER_THAN_USABLE_SIZE;
  return (io_int_t)fread(buffer_addr, sizeof(uint8_t), max_bytes, ((file_buffer_t * const)caller_info)->base_file);
}

static int RefillBufferFromFile(file_buffer_t * const file_buffer)
{
  return RefillBuffer(file_buffer->io_buffer, file_buffer->type == FBT_FILE ? &RefillFromFile : NULL, file_buffer);
}

static io_int_t FlushToFile(const void * const buffer_addr, const size_t num_bytes, void * const caller_info)
{
  if (num_bytes > MAX_USABLE_SIZE)
    return ERROR_VALUE_LARGER_THAN_USABLE_SIZE;
  return (io_int_t)fwrite(buffer_addr, sizeof(uint8_t), num_bytes, ((file_buffer_t * const)caller_info)->base_file);
}

static int FlushBufferToFile(file_buffer_t * const file_buffer)
{
  return FlushBuffer(file_buffer->io_buffer, file_buffer->type == FBT_FILE ? &FlushToFile : NULL, file_buffer);
}

static int AllocateAndInitInternalBuffer(file_buffer_t * const file_buffer, const size_t buffer_size)
{
  int ret;
  if ((file_buffer->io_buffer = AllocateBuffer()) == NULL)
    return ERROR_MEMORY;
  if ((ret = InitBuffer(file_buffer->io_buffer, buffer_size)) != 0)
  {
    FreeBuffer(file_buffer->io_buffer);
    return ret;
  }
  return NO_ERROR;
}

int InitFileBuffer(file_buffer_t * const file_buffer, FILE * const input_file, const file_buffer_mode_t mode, const size_t buffer_size)
{
  int ret;
  if ((ret = AllocateAndInitInternalBuffer(file_buffer, buffer_size)) != NO_ERROR)
    return ret;
  file_buffer->mode = mode;
  file_buffer->base_file = input_file;
  file_buffer->type = FBT_FILE;
  if (file_buffer->mode == FBM_READING)
  {
    if ((ret = RefillBufferFromFile(file_buffer)) != NO_ERROR)
      return ret;
  }
  return NO_ERROR;
}

int InitFileBufferInMemory(file_buffer_t * const file_buffer, const file_buffer_mode_t mode, const size_t buffer_size)
{
  int ret;
  if ((ret = AllocateAndInitInternalBuffer(file_buffer, buffer_size)) != NO_ERROR)
    return ret;
  file_buffer->mode = mode;
  file_buffer->base_file = NULL;
  file_buffer->type = FBT_MEMORY;
  return NO_ERROR;
}

void UninitFileBuffer(file_buffer_t * const file_buffer)
{
  if (file_buffer->mode == FBM_WRITING)
    FlushBufferToFile(file_buffer); /* Ignore result */
  UninitBuffer(file_buffer->io_buffer);
  FreeBuffer(file_buffer->io_buffer);
}

file_buffer_mode_t GetFileBufferMode(const file_buffer_t * const file_buffer)
{
  return file_buffer->mode;
}

int SetFileBufferMode(file_buffer_t * const file_buffer, const file_buffer_mode_t mode)
{
  if (file_buffer->mode == mode)
    return NO_ERROR;
  if ((file_buffer->mode == FBM_WRITING && mode == FBM_READING) || (file_buffer->mode == FBM_READING && mode == FBM_WRITING))
  {
    file_buffer->mode = mode;
    return NO_ERROR;
  }
  else
    return ERROR_INVALID_MODE;
}

size_t GetFileBufferSize(const file_buffer_t * const file_buffer)
{
  return GetBufferSize(file_buffer->io_buffer);
}

int EndOfFileBuffer(file_buffer_t * const file_buffer)
{
  if (file_buffer->mode != FBM_READING) /* There can be no EOF when not in reading mode */
    return ERROR_INVALID_MODE;
  if (GetUsedBufferSize(file_buffer->io_buffer) == 0)
  {
    int ret;
    if ((ret = RefillBufferFromFile(file_buffer)) != NO_ERROR)
      return ret;
  }
  return GetUsedBufferSize(file_buffer->io_buffer) == 0;
}

io_int_t GetActualFileOffset(const file_buffer_t * const file_buffer)
{
  if (file_buffer->type != FBT_FILE)
    return GetUsedBufferSize(file_buffer->io_buffer);
  else
  {
    switch (file_buffer->mode)
    {
      case FBM_READING:
        return FTELL(file_buffer->base_file) - GetUsedBufferSize(file_buffer->io_buffer);
      case FBM_WRITING:
        return FTELL(file_buffer->base_file) + GetUsedBufferSize(file_buffer->io_buffer);
      default:
        return ERROR_INVALID_MODE;
    }
  }
}

io_int_t GetActualFileSize(const file_buffer_t * const file_buffer)
{
  if (file_buffer->mode != FBM_READING) /* There is no file size when not in reading mode */
    return ERROR_INVALID_MODE;
  if (file_buffer->type != FBT_FILE)
    return GetUsedBufferSize(file_buffer->io_buffer);
  else
  {
    const io_uint_t curent_offset = FTELL(file_buffer->base_file); /* Save current file offset */
    io_uint_t ret_val;
    FSEEK(file_buffer->base_file, 0, SEEK_END); /* Go to EOF */
    ret_val = (io_int_t)FTELL(file_buffer->base_file);
    FSEEK(file_buffer->base_file, curent_offset, SEEK_SET); /* Go back to original file offset */
    return ret_val;
  }
}

/* TODO: Implement PeekFileBuffer */

io_int_t ReadFileBuffer(file_buffer_t * const file_buffer, uint8_t * const output, const size_t output_size)
{
  size_t read = 0;
  io_int_t ret = 0;
  if (file_buffer->mode != FBM_READING) /* No peeking or reading when not in reading mode */
    return ERROR_INVALID_MODE;
  if (output_size > MAX_USABLE_SIZE)
    return ERROR_VALUE_LARGER_THAN_USABLE_SIZE;
  while (output_size - read > 0 && (ret = ReadBuffer(file_buffer->io_buffer, &output[0] + read, output_size - read)) != (io_int_t)(output_size - read))
  {
    if (ret == 0 && EndOfFileBuffer(file_buffer)) /* Note: EOF check is necessary since checking only for ret > 0 may fail if the buffer is empty, but EOF is not yet reached */
      break;
    if (ret > 0)
      read += (size_t)ret;
    if ((ret = RefillBufferFromFile(file_buffer)) != NO_ERROR)
      return ret;
  }
  return read + ret; /* ret bytes were read in the last iteration, the rest is from previous iterations */
}

#define RESIZE_BUFFER() { \
  int buf_ret; \
  if ((buf_ret = ResizeBuffer(file_buffer->io_buffer, 2 * GetBufferSize(file_buffer->io_buffer))) != NO_ERROR) /*Double buffer size and retry*/ \
    return (io_int_t)buf_ret; \
}

io_int_t WriteFileBuffer(file_buffer_t * const file_buffer, const uint8_t * const input, const size_t input_size)
{
  size_t written = 0;
  io_int_t old_size, ret = 0;
  if (file_buffer->mode != FBM_WRITING) /* No writing when not in writing mode */
    return ERROR_INVALID_MODE;
  if (input_size > MAX_USABLE_SIZE)
    return ERROR_VALUE_LARGER_THAN_USABLE_SIZE;
  if ((old_size = GetUsedBufferSize(file_buffer->io_buffer)) == (io_int_t)GetBufferSize(file_buffer->io_buffer)) /* Flush or resize buffer if full (prevents unwriteable full buffer when input_size is 1 and buffer size is greater than 1) */
  {
    if (file_buffer->type == FBT_FILE) /* Flush to file */
    {
      if (FlushBufferToFile(file_buffer) != NO_ERROR)
        return ERROR_FILE_IO; /* Nothing was written => abort */
    }
    else /* Resize buffer */
    {
      RESIZE_BUFFER();
    }
  }
  while (input_size - written > 0 && (ret = WriteBuffer(file_buffer->io_buffer, input + written, input_size - written)) != (io_int_t)(input_size - written) && ret > 0)
  {
    if (ret > 0)
      written += (size_t)ret;
    if (file_buffer->type == FBT_FILE) /* Flush to file */
    {
      old_size = GetUsedBufferSize(file_buffer->io_buffer);
      if ((ret = FlushBufferToFile(file_buffer)) != old_size)
        return written - (old_size - ret); /* old_size bytes were expected to be written, but only ret were actually written => abort and report the total actual number written so far (this could be handled differently to allow partial flushing) */
    }
    else /* Resize buffer */
    {
      RESIZE_BUFFER();
    }
  }
  return written + ret; /* ret bytes weren written in the last iteration, the rest is from previous iterations */
}

int FlushFileBuffer(file_buffer_t * const file_buffer)
{
  io_int_t old_size = GetUsedBufferSize(file_buffer->io_buffer);
  if (file_buffer->type != FBT_FILE)
    return ERROR_FILE_IO;
  return FlushBufferToFile(file_buffer) == old_size;
}

int ClearFileBuffer(file_buffer_t * const file_buffer)
{
  if (file_buffer->type != FBT_MEMORY)
    return ERROR_FILE_IO;
  ClearBuffer(file_buffer->io_buffer);
  return NO_ERROR;
}

int ResetFileBuffer(file_buffer_t * const file_buffer, const file_buffer_mode_t mode)
{
  int ret;
  file_buffer->mode = mode;
  if ((ret = ClearFileBuffer(file_buffer)) != NO_ERROR)
    return ret;
  return NO_ERROR;
}