/* Value (de-)normalizer
   Part of DataCompressor
   Andreas Unterweger, 2015 */

#include "err_codes.h"
#include "io_macros.h"
#include "normalize.h"

io_int_t Normalize(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  while (!EndOfBitFileBuffer(in_bit_buf))
  {
    float value;
    const size_t value_size = 8 * sizeof(value);
    io_int_t normalized_value;
    READ_BITS_CHECKED((uint8_t * const)&value, value_size, in_bit_buf, options->error_log_file);
    if (value > 0)
      value = (value * options->normalization_factor + (float)0.5); /* Round towards +inf */
    else if (value < 0)
      value = (value * options->normalization_factor - (float)0.5); /* Round towards -inf */
    if (value < (-(float)((io_uint_t)1 << (options->value_size_bits - 1))) || value > (((io_uint_t)1 << (options->value_size_bits - 1)) - 1)) /* Value range check */
      return ERROR_INVALID_VALUE;
    normalized_value = (io_int_t)value;
    WRITE_VALUE_BITS_CHECKED((const io_uint_t * const)&normalized_value, options->value_size_bits, out_bit_buf, options->error_log_file);
  }
  return NO_ERROR;
}

io_int_t Denormalize(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  while (!EndOfBitFileBuffer(in_bit_buf))
  {
    io_int_t normalized_value;
    float denormalized_value;
    const size_t denormalized_value_size = 8 * sizeof(denormalized_value);
    READ_VALUE_BITS_CHECKED((io_uint_t * const)&normalized_value, options->value_size_bits, in_bit_buf, options->error_log_file);
    normalized_value = EXTEND_IO_INT_SIGN(normalized_value, options->value_size_bits);
    denormalized_value = (float)normalized_value / options->normalization_factor;
    WRITE_BITS_CHECKED((const uint8_t * const)&denormalized_value, denormalized_value_size, out_bit_buf, options->error_log_file);
  }
  return NO_ERROR;
}