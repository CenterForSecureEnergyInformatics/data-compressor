/* Differential coder
   Part of DataCompressor
   Andreas Unterweger, 2015 */

#include "err_codes.h"
#include "io_macros.h"
#include "diff.h"

io_int_t EncodeDifferential(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  io_int_t last_value = 0;
  while (!EndOfBitFileBuffer(in_bit_buf))
  {
    io_int_t value, diff_value;
    READ_VALUE_BITS_CHECKED((io_uint_t * const)&value, options->value_size_bits, in_bit_buf, options->error_log_file);
    diff_value = value - last_value;
    if (options->value_size_bits != IO_SIZE_BITS && (diff_value < (-(io_int_t)((io_uint_t)1 << (options->value_size_bits - 1))) || diff_value > ((((io_int_t)1 << (options->value_size_bits - 1)) - 1)))) /* Value range check */
      return ERROR_INVALID_VALUE;
    WRITE_VALUE_BITS_CHECKED((io_uint_t * const)&diff_value, options->value_size_bits, out_bit_buf, options->error_log_file);
    last_value = value;
  }
  return NO_ERROR;
}

io_int_t DecodeDifferential(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  io_int_t last_value = 0;
  while (!EndOfBitFileBuffer(in_bit_buf))
  {
    io_int_t value;
    READ_VALUE_BITS_CHECKED((io_uint_t * const)&value, options->value_size_bits, in_bit_buf, options->error_log_file);
    value = EXTEND_IO_INT_SIGN(value, options->value_size_bits);
    value += last_value;
    WRITE_VALUE_BITS_CHECKED((const io_uint_t * const)&value, options->value_size_bits, out_bit_buf, options->error_log_file);
    last_value = value;
  }
  return NO_ERROR;
}