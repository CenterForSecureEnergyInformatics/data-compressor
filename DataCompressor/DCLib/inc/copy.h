/* Passthrough ("copy") encoder/decoder (header)
   Part of DataCompressor
   Andreas Unterweger, 2013 */

#ifndef _COPY_H
#define _COPY_H

#include "bit_file_buffer.h"
#include "enc_dec.h"

io_int_t Copy(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options);

#endif