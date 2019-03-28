/* Error codes (header only)
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#ifndef _ERR_CODES_H
#define _ERR_CODES_H

#define NO_ERROR 0

#define ERROR_INVALID_VALUE -1
#define ERROR_VALUE_LARGER_THAN_USABLE_SIZE -2
#define ERROR_INVALID_FORMAT -3
#define ERROR_INVALID_MODE -4
#define ERROR_FILE_IO -5
#define ERROR_MEMORY -6

#define MAX_COMMON_ERROR NO_ERROR
#define MIN_COMMON_ERROR ERROR_MEMORY

static const char * const _common_error_messages[] = { "Successful",
                                                       "Invalid value",
                                                       "Value larger than usable size",
                                                       "Invalid format",
                                                       "Invalid mode",
                                                       "File I/O error",
                                                       "Memory error" };

#define ERROR_LIBRARY_INIT -10
#define ERROR_LIBRARY_CALL -11

#define MAX_LIBRARY_ERROR ERROR_LIBRARY_INIT
#define MIN_LIBRARY_ERROR ERROR_LIBRARY_CALL

static const char * const _library_error_messages[] = { "Error initializing library",
                                                        "Error calling library" };

#define ERROR_MESSAGE_STRING(err_code) \
  ((err_code) >= MIN_COMMON_ERROR && ((err_code) <= MAX_COMMON_ERROR) ? _common_error_messages[MAX_COMMON_ERROR - err_code] : ( \
  ((err_code) >= MIN_LIBRARY_ERROR && ((err_code) <= MAX_LIBRARY_ERROR) ? _library_error_messages[MAX_LIBRARY_ERROR - err_code] : "Unknown error" )))

/* Largest possible error number (8-bit signed int) */
#define MAX_ERROR -128

#endif