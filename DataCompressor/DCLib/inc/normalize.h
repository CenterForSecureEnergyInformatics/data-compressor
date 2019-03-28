/* Value (de-)normalizer (header)
   Part of DataCompressor
   Andreas Unterweger, 2015 */

#ifndef _NORMALIZE_H
#define _NORMALIZE_H

#include "bit_file_buffer.h"
#include "enc_dec.h"

io_int_t Normalize(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options);
io_int_t Denormalize(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options);

#endif