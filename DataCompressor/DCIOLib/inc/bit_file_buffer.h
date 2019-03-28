/* Bit granularity file I/O buffer (header)
   Part of DataCompressor
   Andreas Unterweger 2013-2015 */

#ifndef _BIT_FILE_BUFFER_H
#define _BIT_FILE_BUFFER_H

#include "file_buffer.h"

typedef struct bit_file_buffer_t bit_file_buffer_t;

bit_file_buffer_t *AllocateBitFileBuffer(void);
void FreeBitFileBuffer(bit_file_buffer_t * const bit_file_buffer);

void InitBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, file_buffer_t * const file_buffer);
void UninitBitFileBuffer(bit_file_buffer_t * const bit_file_buffer);

int EndOfBitFileBuffer(const bit_file_buffer_t * const bit_file_buffer);
void GetActualBitFileOffset(const bit_file_buffer_t * const bit_file_buffer, io_int_t * const byte_offset, uint8_t * const bit_offset);
void GetActualBitFileSize(const bit_file_buffer_t * const bit_file_buffer, io_int_t * const byte_offset, uint8_t * const bit_offset);

int SetBitFileBufferMode(bit_file_buffer_t * const bit_file_buffer, const file_buffer_mode_t mode);

io_int_t ReadBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, uint8_t * const output, const size_t output_bit_size);
io_int_t WriteBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, const uint8_t * const input, const size_t input_bit_size);

io_int_t ReadSingleValueFromBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, io_uint_t * const value, const size_t value_bit_size);
io_int_t WriteSingleValueToBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, const io_uint_t * const value, const size_t value_bit_size);

int FlushBitFileBuffer(bit_file_buffer_t * const bit_file_buffer);

int ClearBitFileBuffer(bit_file_buffer_t * const bit_file_buffer);
int ResetBitFileBuffer(bit_file_buffer_t * const bit_file_buffer, const file_buffer_mode_t mode);

#endif