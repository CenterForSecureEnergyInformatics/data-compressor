/* Differential coder (header)
   Part of DataCompressor
   Andreas Unterweger, 2015 */

#ifndef _DIFF_H
#define _DIFF_H

#include "bit_file_buffer.h"
#include "enc_dec.h"

io_int_t EncodeDifferential(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options);
io_int_t DecodeDifferential(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options);

#endif