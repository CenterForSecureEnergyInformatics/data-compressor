/* File I/O buffer (header)
   Part of DataCompressor
   Andreas Unterweger 2013-2015 */

#ifndef _FILE_BUFFER_H
#define _FILE_BUFFER_H

#include "buffer.h"

typedef enum file_buffer_mode_t
{
  FBM_INVALID = -1,
  FBM_READING = 0,
  FBM_WRITING = 1
} file_buffer_mode_t;

typedef struct file_buffer_t file_buffer_t;

file_buffer_t *AllocateFileBuffer(void);
void FreeFileBuffer(file_buffer_t * const file_buffer);

int InitFileBuffer(file_buffer_t * const file_buffer, FILE * const input_file, const file_buffer_mode_t mode, const size_t buffer_size);
int InitFileBufferInMemory(file_buffer_t * const file_buffer, const file_buffer_mode_t mode, const size_t buffer_size);
void UninitFileBuffer(file_buffer_t * const file_buffer);

file_buffer_mode_t GetFileBufferMode(const file_buffer_t * const file_buffer);
int SetFileBufferMode(file_buffer_t * const file_buffer, const file_buffer_mode_t mode);

size_t GetFileBufferSize(const file_buffer_t * const file_buffer);

int EndOfFileBuffer(file_buffer_t * const file_buffer);
io_int_t GetActualFileOffset(const file_buffer_t * const file_buffer);
io_int_t GetActualFileSize(const file_buffer_t * const file_buffer);

io_int_t ReadFileBuffer(file_buffer_t * const file_buffer, uint8_t * const output, const size_t output_size);
io_int_t WriteFileBuffer(file_buffer_t * const file_buffer, const uint8_t * const input, const size_t input_size);

int FlushFileBuffer(file_buffer_t * const file_buffer);

int ClearFileBuffer(file_buffer_t * const file_buffer);
int ResetFileBuffer(file_buffer_t * const file_buffer, const file_buffer_mode_t mode);

#endif
