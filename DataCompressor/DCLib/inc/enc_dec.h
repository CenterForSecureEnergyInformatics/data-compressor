/* Encoders and decoders (header)
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#ifndef _ENC_DEC_H
#define _ENC_DEC_H

#include "bit_file_buffer.h"

typedef struct options_t options_t; /* Forward declaration for settings parameter type */
typedef io_int_t enc_dec_function_t(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options);

typedef struct enc_dec_t
{
  enc_dec_function_t * const encoder;
  enc_dec_function_t * const decoder;
} enc_dec_t;

typedef enum option_type_t
{
  OT_INVALID = 0,
  OT_BOOL,
  OT_SIZE,
  OT_FLOAT,
  OT_CHAR
} option_type_t;

struct options_t
{
  FILE *error_log_file;
  int encode;
  const enc_dec_t *encoder_decoder;
  size_t block_size_bits;
  size_t value_size_bits;
  int adaptive;
  size_t column;
  char separator_char;
  size_t num_decimal_places;
  float normalization_factor;
  size_t num_values;
};

size_t GetNumberOfEncoders(void);
void GetEncoderNames(const char ** const encoder_names);

const enc_dec_t *GetEncoder(const char * const name);
const char * GetEncoderDescription(const char * const name);
const char *GetEncoderNameFromFunction(enc_dec_function_t * const function, const int encoder);

size_t GetNumberOfOptions(void);
void GetOptionNames(const char ** const option_names);
int OptionNameExists(const char * const name);

const char * GetOptionDescription(const char * const name);
option_type_t GetOptionType(const char * const name);
int GetAllowedOptionValueRange(const char * const name, int * const restricted, size_t * const min, size_t * const max);

int EncoderSupportsOption(const char * const name, const char * const option_name);
int EncoderFromFunctionSupportsOption(enc_dec_function_t * const function, int encoder, const char * const option_name);

void SetDefaultOptions(options_t * const options);

int GetOptionValueBool(const options_t * const options, const char * const name, int * const value);
int GetOptionValueSize(const options_t * const options, const char * const name, size_t * const value);
int GetOptionValueFloat(const options_t * const options, const char * const name, float * const value);
int GetOptionValueChar(const options_t * const options, const char * const name, char * const value);

int SetOptionValueBool(options_t * const options, const char * const name, const int value);
int SetOptionValueSize(options_t * const options, const char * const name, const size_t value);
int SetOptionValueFloat(options_t * const options, const char * const name, const float value);
int SetOptionValueChar(options_t * const options, const char * const name, const char value);

#endif