/* Binary arithmetic coder (header)
   Part of DataCompressor
   Andreas Unterweger, 2015 */

#ifndef _BAC_H
#define _BAC_H

#include "bit_file_buffer.h"
#include "enc_dec.h"

io_int_t EncodeBAC(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options);
io_int_t DecodeBAC(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options);

#endif