/* Signed exponential Golomb encoder/decoder (header)
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#ifndef _SEG_H
#define _SEG_H

#include "bit_file_buffer.h"
#include "enc_dec.h"

io_int_t EncodeSEG(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options);
io_int_t DecodeSEG(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options);

#endif