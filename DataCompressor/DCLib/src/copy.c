/* Passthrough ("copy") encoder/decoder
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#include "err_codes.h"
#include "io_macros.h"
#include "copy.h"

#include <stdlib.h>

io_int_t Copy(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  const size_t num_bits = options->block_size_bits;
  const size_t num_bytes = (options->block_size_bits + 7) / 8; /* Always round towards +inf */
  const size_t buf_size = (size_t)(sizeof(uint8_t) * num_bytes);
  uint8_t * const buffer = (uint8_t*)malloc(buf_size);
  if (buffer == NULL)
  {
    LOG(options->error_log_file, "Error allocating %" SIZE_T_FORMAT " bytes. Use a smaller block size\n", SIZE_T_CAST(buf_size));
    return ERROR_MEMORY;
  }
  while (!EndOfBitFileBuffer(in_bit_buf))
  {
    READ_BITS_CHECKED_WITH_ACTION_ON_ERROR(buffer, num_bits, in_bit_buf, options->error_log_file, free(buffer));
    WRITE_BITS_CHECKED_WITH_ACTION_ON_ERROR(buffer, num_bits, out_bit_buf, options->error_log_file, free(buffer));
  }
  free(buffer);
  return NO_ERROR;
}