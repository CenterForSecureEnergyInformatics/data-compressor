/* Signed exponential Golomb encoder/decoder
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#include "err_codes.h"
#include "io_macros.h"
#include "seg.h"

static const io_uint_t zeros[1] = { 0 };

static io_int_t EncodeUEGCodeword(const io_uint_t value, bit_file_buffer_t * const out_bit_buf, FILE * const error_log_file)
{
  const io_uint_t value_plus_one = value + 1;
  io_uint_t temp_value = value_plus_one;
  size_t prefix_length = 0;
  while ((temp_value >>= 1) != 0) /* Determine prefix length */
    prefix_length++;
  WRITE_VALUE_BITS_CHECKED(zeros, prefix_length, out_bit_buf, error_log_file); /* Write prefix zeros */
  WRITE_VALUE_BITS_CHECKED(&value_plus_one, 1 + prefix_length, out_bit_buf, error_log_file); /* Write delimiting one and postfix value residual */
  return NO_ERROR;
}

static io_int_t EncodeSEGCodeword(const io_int_t value, bit_file_buffer_t * const out_bit_buf, FILE * const error_log_file)
{
  if (value > 0)
    return EncodeUEGCodeword(2 * (io_uint_t)IO_ABS(value) - 1, out_bit_buf, error_log_file);
  else /* if (value <= 0) */
    return EncodeUEGCodeword(2 * (io_uint_t)IO_ABS(value), out_bit_buf, error_log_file);
}

io_int_t EncodeSEG(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  io_int_t ret;
  while (!EndOfBitFileBuffer(in_bit_buf))
  {
    io_int_t current_value;
    READ_VALUE_BITS_CHECKED((io_uint_t * const)&current_value, options->value_size_bits, in_bit_buf, options->error_log_file);
    current_value = EXTEND_IO_INT_SIGN(current_value, options->value_size_bits);
    if ((ret = EncodeSEGCodeword(current_value, out_bit_buf, options->error_log_file)) != NO_ERROR)
      return ret;
  }
  return NO_ERROR;
}

static io_int_t DecodeUEGCodeword(const size_t max_value_size_bits, io_uint_t * const value, int * const eos, bit_file_buffer_t * const in_bit_buf, FILE * const error_log_file)
{
  size_t prefix_length = 0;
  io_uint_t current_bit = 0;
  *eos = 0;
  while (current_bit == 0 && !EndOfBitFileBuffer(in_bit_buf)) /* Read prefix */
  {
    READ_VALUE_BITS_CHECKED(&current_bit, (size_t)1, in_bit_buf, error_log_file);
    if (current_bit == 0)
      prefix_length++;
    if (prefix_length >= max_value_size_bits) /* Max. prefix length for 64 bit values is 64 */
      return ERROR_INVALID_FORMAT;
  }
  if (EndOfBitFileBuffer(in_bit_buf) && prefix_length != 0) /* EOF during prefix */
  {
    *eos = 1; /* Signal end of stream and abort */
    return NO_ERROR;
  }
  *value = 0;
  READ_VALUE_BITS_CHECKED(value, prefix_length, in_bit_buf, error_log_file); /* Read postfix value residual */
  *value |= (io_uint_t)1 << prefix_length;
  *value = *value - 1;
  return NO_ERROR;
}

static io_int_t DecodeSEGCodeword(const size_t max_value_size_bits, io_int_t * const value, int * const eos, bit_file_buffer_t * const in_bit_buf, FILE * const error_log_file)
{
  io_int_t ret;
  io_uint_t abs_value;
  if (((ret = DecodeUEGCodeword(max_value_size_bits + 1 > IO_SIZE_BITS ? IO_SIZE_BITS : max_value_size_bits + 1, &abs_value, eos, in_bit_buf, error_log_file)) != NO_ERROR) || *eos) /* SEG code words are one bit longer */
    return ret;
  *value = (abs_value + 1) / 2;
  if ((abs_value & 1) == 0) /* Even code words encode negative values (odd code words encode positive values) */
    *value = -*value;
  return NO_ERROR;
}

io_int_t DecodeSEG(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  while (!EndOfBitFileBuffer(in_bit_buf))
  {
    io_int_t current_value, ret;
    int eos;
    if ((ret = DecodeSEGCodeword(options->value_size_bits, &current_value, &eos, in_bit_buf, options->error_log_file)) != NO_ERROR)
      return ret;
    if (eos) /* Exit on EOS indicator */
      break;
    WRITE_VALUE_BITS_CHECKED((const io_uint_t * const)&current_value, options->value_size_bits, out_bit_buf, options->error_log_file);
  }
  return NO_ERROR;
}