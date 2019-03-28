/* Value aggregator
   Part of DataCompressor
   Andreas Unterweger, 2015 */

#include "err_codes.h"
#include "io_macros.h"
#include "aggregate.h"

io_int_t Aggregate(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  while (!EndOfBitFileBuffer(in_bit_buf))
  {
    float sum = 0;
    const size_t value_size = 8 * sizeof(sum);
    size_t i;
    for (i = 0; i < options->num_values; i++)
    {
      float value;
      READ_BITS_CHECKED((uint8_t * const)&value, value_size, in_bit_buf, options->error_log_file);
      sum += value;
      if (EndOfBitFileBuffer(in_bit_buf)) /* Stop aggregating if there are no more values */
        break;
    }
    WRITE_BITS_CHECKED((const uint8_t * const)&sum, value_size, out_bit_buf, options->error_log_file);
  }
  return NO_ERROR;
}