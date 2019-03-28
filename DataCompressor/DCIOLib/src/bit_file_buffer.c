/* Bit-granularity file I/O buffer
   Part of DataCompressor
   Andreas Unterweger 2013-2015 */

#include "err_codes.h"
#include "bit_file_buffer.h"

#include <stdlib.h>

#define MAX_USED_BITS 8

struct bit_file_buffer_t
{
  file_buffer_t *file_buffer;
  uint8_t byte_buffer;
  uint8_t used_bits;
  uint8_t extra_byte_buffer; /* Used when swapping from writing to reading mode */
  uint8_t extra_used_bits;
};

bit_file_buffer_t *AllocateBitFileBuffer(void)
{
  return (bit_file_buffer_t*)malloc(sizeof(bit_file_buffer_t));
}

void FreeBitFileBuffer(bit_file_buffer_t * const bit_file_buffer)
{
  free(bit_file_buffer);
}

static void ResetBufferByte(bit_file_buffer_t * const bit_file_buffer)
{
  bit_file_buffer->byte_buffer = 0;
  switch (GetFileBufferMode(bit_file_buffer->file_buffer))
  {
    case FBM_READING:
      bit_file_buffer->used_bits = MAX_USED_BITS;
      break;
    case FBM_WRITING:
    default:
      bit_file_buffer->used_bits = 0;
      break;
  }
  bit_file_buffer->extra_byte_buffer = bit_file_buffer->extra_used_bits = 0;
}

void InitBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, file_buffer_t * const file_buffer)
{
  bit_file_buffer->file_buffer = file_buffer;
  ResetBufferByte(bit_file_buffer);
}

int EndOfBitFileBuffer(const bit_file_buffer_t * const bit_file_buffer)
{
  const int eof = EndOfFileBuffer(bit_file_buffer->file_buffer);
  if (eof < 0) /* Error */
    return eof;
  if (GetFileBufferMode(bit_file_buffer->file_buffer) == FBM_READING && eof)
    return bit_file_buffer->extra_used_bits == 0 && bit_file_buffer->used_bits == MAX_USED_BITS;
  else
    return 0;
}

void GetActualBitFileOffset(const bit_file_buffer_t * const bit_file_buffer, io_int_t * const byte_offset, uint8_t * const bit_offset)
{
  const io_int_t offset = GetActualFileOffset(bit_file_buffer->file_buffer);
  if (offset < 0)
  {
    *byte_offset = offset;
    *bit_offset = 0;
    return;
  }
  switch (GetFileBufferMode(bit_file_buffer->file_buffer))
  {
    case FBM_READING:
      if (bit_file_buffer->used_bits == MAX_USED_BITS)
      {
        *byte_offset = offset;
        *bit_offset = 0;
      }
      else
      {
        *byte_offset = offset - 1;
        *bit_offset = bit_file_buffer->used_bits;
      }
      break;
    case FBM_WRITING:
      if (bit_file_buffer->used_bits == MAX_USED_BITS)
      {
        *byte_offset = offset + 1;
        *bit_offset = 0;
      }
      else
      {
        *byte_offset = offset;
        *bit_offset = bit_file_buffer->used_bits;
      }
      break;
    default:
      *byte_offset = ERROR_INVALID_MODE;
      *bit_offset = 0;
      break;
  }
}

void GetActualBitFileSize(const bit_file_buffer_t * const bit_file_buffer, io_int_t * const byte_size, uint8_t * const bit_size)
{
  const io_int_t size = GetActualFileSize(bit_file_buffer->file_buffer);
  if (size < 0)
  {
    *byte_size = size;
    *bit_size = 0;
    return;
  }
  if (bit_file_buffer->extra_used_bits == MAX_USED_BITS)
  {
    *byte_size = size + 1;
    *bit_size = 0;
  }
  else
  {
    *byte_size = size;
    *bit_size = bit_file_buffer->extra_used_bits;
  }
}

int SetBitFileBufferMode(bit_file_buffer_t * const bit_file_buffer, const file_buffer_mode_t mode)
{
  const file_buffer_mode_t old_mode = GetFileBufferMode(bit_file_buffer->file_buffer);
  int ret;
  if ((ret = SetFileBufferMode(bit_file_buffer->file_buffer, mode)) != NO_ERROR)
    return ret;
  if (old_mode == mode)
    return NO_ERROR;
  else if (old_mode == FBM_WRITING && mode == FBM_READING) /* Changed from writing to reading (the other way around is not supported) */
  {
    bit_file_buffer->extra_byte_buffer = bit_file_buffer->byte_buffer;
    bit_file_buffer->extra_used_bits = bit_file_buffer->used_bits;
    bit_file_buffer->used_bits = MAX_USED_BITS; /* Mark all bits as used => force next read operation to refill buffer */
    return NO_ERROR;
  }
  else
    return ERROR_INVALID_MODE;
}

/* TODO: Implement PeekBitFileBuffer */

static int8_t ReadBitFileBufferBytewise(bit_file_buffer_t * const bit_file_buffer, uint8_t * const output, const uint8_t output_bit_size)
{
  uint8_t new_bits = output_bit_size;
  int8_t read = 0;
  const uint8_t old_bits = MAX_USED_BITS - bit_file_buffer->used_bits;
  if (output_bit_size > MAX_USED_BITS)
    return ERROR_INVALID_VALUE;
  *output = 0;
  if (output_bit_size > old_bits)
  {
    if (old_bits > 0)
    {
      *output |= bit_file_buffer->byte_buffer & ((1 << old_bits) - 1); /* Read remaining bits */
      read += old_bits;
      bit_file_buffer->used_bits = MAX_USED_BITS;
    }
    new_bits -= read;
    if (ReadFileBuffer(bit_file_buffer->file_buffer, &bit_file_buffer->byte_buffer, 1) == 1) /* Refill buffer (byte) */
      bit_file_buffer->used_bits = 0;
    else /* No more bits to be read (possibly EOF) => abort and return what could be read (except if there are left-over bits) */
    {
      if (bit_file_buffer->extra_used_bits > 0) /* Special case: changed from writing to reading mode and there are left-over bits */
      {
        if (new_bits > bit_file_buffer->extra_used_bits) /* Don't read more bits than are available */
          new_bits = bit_file_buffer->extra_used_bits;
        bit_file_buffer->byte_buffer = bit_file_buffer->extra_byte_buffer >> (MAX_USED_BITS - bit_file_buffer->extra_used_bits);
        bit_file_buffer->used_bits = MAX_USED_BITS - bit_file_buffer->extra_used_bits;
        bit_file_buffer->extra_used_bits = 0;
      }
      else
        return read; /* Abort and return */
    }
    if (old_bits > 0)
      *output <<= output_bit_size - old_bits;
  }
  *output |= (bit_file_buffer->byte_buffer & ((1 << (MAX_USED_BITS - bit_file_buffer->used_bits)) - 1)) >> (MAX_USED_BITS - new_bits - bit_file_buffer->used_bits); /* Read bits (MSB first) */
  bit_file_buffer->used_bits += new_bits;
  read += new_bits;
  return read;
}

io_int_t ReadBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, uint8_t * const output, const size_t output_bit_size)
{
  const size_t num_full_reads = output_bit_size / MAX_USED_BITS;
  uint8_t remaining_bits = output_bit_size % MAX_USED_BITS;
  if (GetFileBufferMode(bit_file_buffer->file_buffer) != FBM_READING) /* Only read when in reading mode */
    return ERROR_INVALID_MODE;
  if (num_full_reads > 0)
  {
    size_t i;
    for (i = 0; i < num_full_reads; i++)
    {
      int8_t read;
      if ((read = ReadBitFileBufferBytewise(bit_file_buffer, &output[i], MAX_USED_BITS)) != MAX_USED_BITS)
      {
        output[i] <<= (MAX_USED_BITS - read); /* Realign fractional byte */
        return i * MAX_USED_BITS + read; /* Return number of (previously) successfully read bytes plus remaining fraction */
      }
    }
  }
  if (remaining_bits > 0)
  {
    uint8_t temp_byte;
    int8_t read;
    if ((read = ReadBitFileBufferBytewise(bit_file_buffer, &temp_byte, remaining_bits)) != remaining_bits)
      return num_full_reads * MAX_USED_BITS + (read >= 0 ? read : 0); /* Return number of (previously) successfully read bytes (include last bits read if there was no error) */
    else
      output[num_full_reads] = temp_byte << (MAX_USED_BITS - remaining_bits);
  }
  return output_bit_size; /* Reading successful */
}

static int8_t WriteBitFileBufferBytewise(bit_file_buffer_t * const bit_file_buffer, const uint8_t * const input, const uint8_t input_bit_size)
{
  int8_t written = 0;
  const int8_t free_bits = MAX_USED_BITS - bit_file_buffer->used_bits;
  int8_t new_bits;
  if (input_bit_size > MAX_USED_BITS)
    return ERROR_INVALID_VALUE;
  if (input_bit_size > free_bits)
  {
    if (free_bits > 0)
    {
      bit_file_buffer->byte_buffer |= (*input & ((1 << input_bit_size) - 1)) >> (input_bit_size - free_bits);
      written += free_bits;
    }
    if (WriteFileBuffer(bit_file_buffer->file_buffer, &bit_file_buffer->byte_buffer, 1) == 1) /* Flush buffer (byte) */
      bit_file_buffer->used_bits = 0;
    else /* Writing was unsuccessful => abort and return what could be written */
    {
      bit_file_buffer->used_bits += written;
      return written;
    }
    bit_file_buffer->byte_buffer = 0; /* Reinitialize buffer (byte) */
  }
  new_bits = input_bit_size - written;
  bit_file_buffer->byte_buffer |= (*input & ((1 << new_bits) - 1)) << (MAX_USED_BITS - new_bits - bit_file_buffer->used_bits); /* Write bits (MSB first) */
  bit_file_buffer->used_bits += new_bits;
  written += new_bits;
  return written;
}

io_int_t WriteBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, const uint8_t * const input, const size_t input_bit_size)
{
  const size_t num_full_writes = input_bit_size / MAX_USED_BITS;
  const uint8_t remaining_bits = input_bit_size % MAX_USED_BITS;
  if (GetFileBufferMode(bit_file_buffer->file_buffer) != FBM_WRITING) /* Only write when in writing mode */
    return ERROR_INVALID_MODE;
  if (num_full_writes > 0)
  {
    size_t i;
    for (i = 0; i < num_full_writes; i++)
    {
      int8_t written;
      if ((written = WriteBitFileBufferBytewise(bit_file_buffer, &input[i], MAX_USED_BITS)) != MAX_USED_BITS)
        return i * MAX_USED_BITS + written; /* Return number of (previously) successfully written bytes plus remaining fraction */
    }
  }
  if (remaining_bits > 0)
  {
    uint8_t temp_byte = input[num_full_writes] >> (MAX_USED_BITS - remaining_bits);
    int8_t written;
    if ((written = WriteBitFileBufferBytewise(bit_file_buffer, &temp_byte, remaining_bits)) != remaining_bits)
      return num_full_writes * MAX_USED_BITS; /* Return number of (previously) successfully written bytes */
  }
  return input_bit_size; /* Writing successful */
}

#define MAX_SIZE (sizeof(io_uint_t))
#define MAX_BIT_SIZE (8 * MAX_SIZE)

/* TODO: Optimize */
io_int_t ReadSingleValueFromBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, io_uint_t * const value, const size_t value_bit_size)
{
  io_int_t ret;
  uint8_t bytes[MAX_SIZE];
  size_t i;
  if (value_bit_size > MAX_BIT_SIZE)
    return ERROR_INVALID_VALUE;
  if ((ret = ReadBitFileBuffer(bit_file_buffer, bytes, value_bit_size)) != (io_int_t)value_bit_size)
    return ret;
  *value = 0;
  for (i = 0; i < (value_bit_size + 7) / 8; i++)
    *value |= (io_int_t)bytes[i] << (MAX_BIT_SIZE - 8 - 8 * i);
  *value >>= (MAX_BIT_SIZE - value_bit_size);
  return value_bit_size;
}

/* TODO: Optimize */
io_int_t WriteSingleValueToBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, const io_uint_t * const value, const size_t value_bit_size)
{
  const io_uint_t temp_value = *value << (MAX_BIT_SIZE - value_bit_size);
  uint8_t bytes[MAX_SIZE];
  size_t i;
  if (value_bit_size > MAX_BIT_SIZE)
    return ERROR_INVALID_VALUE;
  bytes[0] = (uint8_t)(temp_value >> (MAX_BIT_SIZE - 8)); /* Treat first byte separately to avoid shifting by 64 below */
  for (i = 1; i < (value_bit_size + 7) / 8; i++)
    bytes[i] = (temp_value & (((io_uint_t)1 << (MAX_BIT_SIZE - 8 * i)) - 1)) >> (MAX_BIT_SIZE - 8 - 8 * i);
  return WriteBitFileBuffer(bit_file_buffer, bytes, value_bit_size);
}

static int FlushBitFileBufferInternal(bit_file_buffer_t * const bit_file_buffer, int WriteFractionalBytes)
{
  if ((!WriteFractionalBytes && bit_file_buffer->used_bits == MAX_USED_BITS) || WriteFractionalBytes) /* Write buffer (byte) */
  {
    if (WriteFileBuffer(bit_file_buffer->file_buffer, &bit_file_buffer->byte_buffer, 1) == 1)
      bit_file_buffer->used_bits = 0;
    else /* Writing was not successful => abort */
      return ERROR_FILE_IO;
  }
  return FlushFileBuffer(bit_file_buffer->file_buffer);
}

int FlushBitFileBuffer(bit_file_buffer_t * const bit_file_buffer)
{
  if (GetFileBufferMode(bit_file_buffer->file_buffer) != FBM_WRITING) /* Only flush when in writing mode */
    return ERROR_INVALID_MODE;
  return FlushBitFileBufferInternal(bit_file_buffer, 0);
}

void UninitBitFileBuffer(bit_file_buffer_t * const bit_file_buffer)
{
  if (GetFileBufferMode(bit_file_buffer->file_buffer) == FBM_WRITING)
    FlushBitFileBufferInternal(bit_file_buffer, 1); /* Ignore result */
}

int ClearBitFileBuffer(bit_file_buffer_t * const bit_file_buffer)
{
  int ret;
  if ((ret = ClearFileBuffer(bit_file_buffer->file_buffer)) != NO_ERROR)
    return ret;
  ResetBufferByte(bit_file_buffer);
  return NO_ERROR;
} 

int ResetBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, const file_buffer_mode_t mode)
{
  int ret;
  if ((ret = ResetFileBuffer(bit_file_buffer->file_buffer, mode)) != NO_ERROR)
    return ret;
  if ((ret = ClearBitFileBuffer(bit_file_buffer)) != NO_ERROR)
    return ret;
  return NO_ERROR;
}