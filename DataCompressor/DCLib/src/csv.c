/* Comma-separated value reader/writer
   Part of DataCompressor
   Andreas Unterweger, 2015 */

#include "err_codes.h"
#include "io_macros.h"
#include "csv.h"

#include <float.h>

#define FLOAT_TEXT_BUFFER_SIZE (1 /*'-'*/ + (FLT_MAX_10_EXP + 1) /*38+1 digits*/ + 1 /*'.'*/ + FLT_DIG /*Default precision*/ + 1 /*\0*/) /* Adopted from http://stackoverflow.com/questions/7235456/what-are-the-maximum-numbers-of-characters-output-by-sprintf-when-outputting-flo */

io_int_t ReadCSV(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  size_t column = 1;
  char current_char;
  const size_t current_char_size = 8 * sizeof(current_char);
  char buffer[FLOAT_TEXT_BUFFER_SIZE] = { 0 };
  size_t used_buffer = 0;
  while (!EndOfBitFileBuffer(in_bit_buf))
  {
    READ_BITS_CHECKED((uint8_t * const)&current_char, current_char_size, in_bit_buf, options->error_log_file);
    if (current_char == options->separator_char || current_char == '\n' || EndOfBitFileBuffer(in_bit_buf))
    {
      if (EndOfBitFileBuffer(in_bit_buf) && column == options->column /* Text character from desired column */)
        buffer[used_buffer++] = current_char;
      if (column == options->column)
      {
        float value;
        const size_t value_size = 8 * sizeof(value);
        buffer[used_buffer++] = '\0'; /* Make sure that the string is terminated */
        value = strtof(buffer, NULL); /* Interpret value as float */
        WRITE_BITS_CHECKED((const uint8_t * const)&value, value_size, out_bit_buf, options->error_log_file);
        used_buffer = 0; /* Reset buffer */
      }
      column++;
    }
    else if (column == options->column) /* Text character from desired column */
      buffer[used_buffer++] = current_char;
    if (current_char == '\n') /* Next line */
      column = 1; /* Reset column */
  }
  return NO_ERROR;
}

io_int_t WriteCSV(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  while (!EndOfBitFileBuffer(in_bit_buf))
  {
    io_int_t retval;
    float value;
    const size_t value_size = 8 * sizeof(value);
    size_t i;
    char buffer[FLOAT_TEXT_BUFFER_SIZE];
    READ_BITS_CHECKED((uint8_t * const)&value, value_size, in_bit_buf, options->error_log_file);
    for (i = 1; i < options->column; i++) /* Create empty columns if necessary */
    {
      WRITE_BITS_CHECKED((const uint8_t * const)&options->separator_char, 8 * sizeof(options->separator_char), out_bit_buf, options->error_log_file);
    }
    if ((retval = (io_int_t)sprintf(buffer, "%.*f\n", (int)options->num_decimal_places, value)) < 0)
      return ERROR_MEMORY;
    WRITE_BITS_CHECKED((const uint8_t * const)buffer, 8 * (size_t)retval, out_bit_buf, options->error_log_file); /* Write number of characters in bytes! */
  }
  return NO_ERROR;
}