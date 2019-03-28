/* Value aggregator (header)
   Part of DataCompressor
   Andreas Unterweger, 2015 */

#ifndef _AGGREGATE_H
#define _AGGREGATE_H

#include "bit_file_buffer.h"
#include "enc_dec.h"

io_int_t Aggregate(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options);

#endif