/* DataCompressor command line interface (header)
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#ifndef _CLI_H
#define _CLI_H

#include "bit_file_buffer.h"

typedef struct buffer_environment_t
{
  FILE *in_file, *out_file;
  file_buffer_t *in_buf, *out_buf, *temp_read_buf, *temp_write_buf;
  bit_file_buffer_t *in_bit_buf, *out_bit_buf, *temp_read_bit_buf, *temp_write_bit_buf;
} buffer_environment_t;

void InitBufferEnvironment(buffer_environment_t * const buffer_env);
void UninitBufferEnvironment(buffer_environment_t * const buffer_env);

int InitBuffers(buffer_environment_t * const buffers, FILE * const error_log_file, const int use_temp_buffers);
void UninitBuffers(buffer_environment_t * const buffers);

int main(const int argc, const char * const * const argv);

#endif