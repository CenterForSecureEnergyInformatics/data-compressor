/* Macros for I/O reading and writing with error handling (header only)
   Part of DataCompressor
   Andreas Unterweger, 2015 */

#ifndef _IO_MACROS_H
#define _IO_MACROS_H

#include "log.h"

#define LOG_IO_ERROR_FORMAT_STRING(read, ret, num_bits) "Only %s %" IO_INT_FORMAT " bits instead of %" SIZE_T_FORMAT "\n", (read) ? "read" : "wrote", (ret), SIZE_T_CAST((num_bits))
#define LOG_ERROR_FORMAT_STRING(read, ret, num_bits) "%s while trying to %s %" SIZE_T_FORMAT " bits\n", ERROR_MESSAGE_STRING(ret), (read) ? "read" : "write", SIZE_T_CAST((num_bits))

#define LOG_ON_ERROR_AND_RETURN(ret, num_bits, error_log_file, read) { \
  if ((ret) != (io_int_t)(num_bits)) \
  { \
    if ((ret) >= 0) \
    { \
      LOG_ERROR((error_log_file), LOG_IO_ERROR_FORMAT_STRING((read), (ret), (num_bits))); \
      return ERROR_LIBRARY_CALL; \
    } \
    else \
    { \
      LOG_ERROR((error_log_file), LOG_ERROR_FORMAT_STRING((read), (ret), (num_bits))); \
      return (ret); \
    } \
  } \
}

#define LOG_ON_ERROR_PERFORM_ACTION_AND_RETURN(ret, num_bits, error_log_file, read, action) { \
  if ((ret) != (io_int_t)(num_bits)) \
  { \
    if ((ret) >= 0) \
    { \
      LOG_ERROR((error_log_file), LOG_IO_ERROR_FORMAT_STRING((read), (ret), (num_bits))); \
      (action); \
      return ERROR_LIBRARY_CALL; \
    } \
    else \
    { \
      LOG_ERROR((error_log_file), LOG_ERROR_FORMAT_STRING((read), (ret), (num_bits))); \
      (action); \
      return (ret); \
    } \
  } \
}

#define READ_BITS_CHECKED(bits, num_bits, in_bit_buf, error_log_file) { \
  { \
    io_int_t ret = ReadBitFileBuffer((in_bit_buf), (bits), (num_bits)); \
    LOG_ON_ERROR_AND_RETURN(ret, (num_bits), (error_log_file), 1); \
  } \
}

#define READ_BITS_CHECKED_WITH_ACTION_ON_ERROR(bits, num_bits, in_bit_buf, error_log_file, action) { \
  { \
    io_int_t ret = ReadBitFileBuffer((in_bit_buf), (bits), (num_bits)); \
    LOG_ON_ERROR_PERFORM_ACTION_AND_RETURN(ret, (num_bits), (error_log_file), 1, (action)); \
  } \
}

#define READ_VALUE_BITS_CHECKED(bits, num_bits, in_bit_buf, error_log_file) { \
  { \
    io_int_t ret = ReadSingleValueFromBitFileBuffer((in_bit_buf), (bits), (num_bits)); \
    LOG_ON_ERROR_AND_RETURN(ret, (num_bits), (error_log_file), 1); \
  } \
}

#define WRITE_BITS_CHECKED(bits, num_bits, out_bit_buf, error_log_file) { \
  { \
    io_int_t ret = WriteBitFileBuffer((out_bit_buf), (bits), (num_bits)); \
    LOG_ON_ERROR_AND_RETURN(ret, (num_bits), (error_log_file), 0); \
  } \
}

#define WRITE_BITS_CHECKED_WITH_ACTION_ON_ERROR(bits, num_bits, out_bit_buf, error_log_file, action) { \
  { \
    io_int_t ret = WriteBitFileBuffer((out_bit_buf), (bits), (num_bits)); \
    LOG_ON_ERROR_PERFORM_ACTION_AND_RETURN(ret, (num_bits), (error_log_file), 0, (action)); \
  } \
}

#define WRITE_VALUE_BITS_CHECKED(bits, num_bits, out_bit_buf, error_log_file) { \
  { \
    io_int_t ret = WriteSingleValueToBitFileBuffer((out_bit_buf), (bits), (num_bits)); \
    LOG_ON_ERROR_AND_RETURN(ret, (num_bits), (error_log_file), 0); \
  } \
}

#define EXTEND_IO_INT_SIGN(value, value_bits) ((value_bits) == IO_SIZE_BITS ? (value) : /* Nothing to extend */ \
  ((value) << (IO_SIZE_BITS - (value_bits))) >> (IO_SIZE_BITS - (value_bits))) /* Extend MSB */

#endif