/* Encoders and decoders
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#include "err_codes.h"
#include "copy.h"
#include "seg.h"
#include "csv.h"
#include "normalize.h"
#include "bac.h"
#include "aggregate.h"
#include "lzmh.h"
#include "diff.h"
#include "enc_dec.h"

#include <string.h>
#include <stddef.h>

typedef enum option_t
{
  NO_OPTIONS = 0,
  OPTION_BLOCK_SIZE_BITS = 1 << 0,
  OPTION_VALUE_SIZE_BITS = 1 << 1,
  OPTION_ADAPTIVE = 1 << 2,
  OPTION_COLUMN = 1 << 3,
  OPTION_SEPARATOR_CHAR = 1 << 4,
  OPTION_NORMALIZATION_FACTOR = 1 << 5,
  OPTION_NUM_DECIMAL_PLACES = 1 << 6,
  OPTION_NUM_VALUES = 1 << 7
} option_t;

typedef struct named_enc_dec_t
{
  const char * const name;
  const char * const description;
  const enc_dec_t encoder_decoder;
  const option_t supported_options;
} named_enc_dec_t;

typedef struct option_description_t
{
  const char * const name;
  const option_t option;
  const char * const description;
  const option_type_t type;
  const size_t min_value;
  const size_t max_value;
  const size_t struct_offset;
} option_description_t;

static const named_enc_dec_t encoders_decoders[] = { /* Note: This array needs to be sorted by name so that binary search works */
  { "aggregate", "Sums up values", { &Aggregate, NULL }, OPTION_NUM_VALUES }, /* No decoder! */
  { "bac", "Binary arithmetic coding", { &EncodeBAC, &DecodeBAC }, OPTION_ADAPTIVE },
  { "copy", "Copies input to output", { &Copy, &Copy }, OPTION_BLOCK_SIZE_BITS },
  { "csv", "Comma-separated values", { &WriteCSV, &ReadCSV }, OPTION_COLUMN | OPTION_SEPARATOR_CHAR | OPTION_NUM_DECIMAL_PLACES },
  { "diff", "Differential coding", { &EncodeDifferential, &DecodeDifferential }, OPTION_VALUE_SIZE_BITS },
  { "lzmh", "LZMH coding", { &EncodeLZMH, &DecodeLZMH }, NO_OPTIONS },
  { "normalize", "(De-)normalization", { &Normalize, &Denormalize }, OPTION_NORMALIZATION_FACTOR | OPTION_VALUE_SIZE_BITS },
  { "seg", "Signed Exponential Golomb coding", { &EncodeSEG, &DecodeSEG }, OPTION_VALUE_SIZE_BITS }
};

static const size_t num_encoders = sizeof(encoders_decoders) / sizeof(encoders_decoders[0]);

static const option_description_t option_descriptions[] = { /* Note: This array needs to be sorted by name so that binary search works */
  { "adaptive", OPTION_ADAPTIVE, "Perform adaptive arithmetic coding", OT_BOOL, 0, 1, offsetof(options_t, adaptive) },
  { "blocksize", OPTION_BLOCK_SIZE_BITS, "Use blocks of <n> bits size for I/O", OT_SIZE, 1, SIZE_MAX, offsetof(options_t, block_size_bits) },
  { "column", OPTION_COLUMN, "Use column <n>", OT_SIZE, 1, SIZE_MAX, offsetof(options_t, column) },
  { "normalization_factor", OPTION_NORMALIZATION_FACTOR, "Use multiplier <n> for normalization and <1/n> for denormalization", OT_FLOAT, 0, SIZE_MAX, offsetof(options_t, normalization_factor) },
  { "num_decimal_places", OPTION_NUM_DECIMAL_PLACES, "Use <n> decimal places to print floats into CSV files", OT_SIZE, 0, 6, offsetof(options_t, num_decimal_places) },
  { "num_values", OPTION_NUM_VALUES, "Use <n> values for aggregation", OT_SIZE, 0, SIZE_MAX, offsetof(options_t, num_values) },
  { "separator_char", OPTION_SEPARATOR_CHAR, "Use <n> as CSV entry separator", OT_CHAR, 0, CHAR_MAX, offsetof(options_t, separator_char) },
  { "valuesize", OPTION_VALUE_SIZE_BITS, "Use values of <n> bits size", OT_SIZE, 1, IO_SIZE_BITS, offsetof(options_t, value_size_bits) }
};

static const size_t num_options = sizeof(option_descriptions) / sizeof(option_descriptions[0]);

size_t GetNumberOfEncoders(void)
{
  return num_encoders;
}

void GetEncoderNames(const char ** const encoder_names)
{
  size_t i;
  for (i = 0; i < num_encoders; i++)
    encoder_names[i] = encoders_decoders[i].name;
}

static int CompareEncoderNames(const void * const pkey, const void * const pelem)
{
  const char * const enc_dec_name = ((const named_enc_dec_t * const)pelem)->name;
  return strncmp((const char*)pkey, enc_dec_name, strlen(enc_dec_name));
}

static named_enc_dec_t *FindEncoder(const char * const name)
{
  return (named_enc_dec_t*)bsearch(name, encoders_decoders, num_encoders, sizeof(named_enc_dec_t), &CompareEncoderNames);
}

const enc_dec_t *GetEncoder(const char * const name)
{
  const named_enc_dec_t * const encoder = FindEncoder(name);
  return encoder == NULL ? NULL : &encoder->encoder_decoder;
}

const char *GetEncoderDescription(const char * const name)
{
  const named_enc_dec_t * const encoder = FindEncoder(name);
  return encoder == NULL ? NULL : encoder->description;
}

const char *GetEncoderNameFromFunction(enc_dec_function_t * const function, const int encoder)
{
  size_t i;
  for (i = 0; i < num_encoders; i++)
  {
    if ((encoder && encoders_decoders[i].encoder_decoder.encoder == function) || (!encoder && encoders_decoders[i].encoder_decoder.decoder == function))
      return encoders_decoders[i].name;
  }
  return NULL;
}

size_t GetNumberOfOptions(void)
{
  return num_options;
}

void GetOptionNames(const char ** const option_names)
{
  size_t i;
  for (i = 0; i < num_options; i++)
    option_names[i] = option_descriptions[i].name;
}

static int CompareOptionNames(const void *const pkey, const void * const pelem)
{
  const char * const option_name = ((const option_description_t * const)pelem)->name;
  return strncmp((const char*)pkey, option_name, strlen(option_name));
}

static const option_description_t *FindOptionDescription(const char * const name)
{
  return (const option_description_t*)bsearch(name, option_descriptions, num_options, sizeof(option_description_t), &CompareOptionNames);
}

int OptionNameExists(const char * const name)
{
  return FindOptionDescription(name) != NULL;
}

const char *GetOptionDescription(const char * const name)
{
  const option_description_t * const option_desc = FindOptionDescription(name);
  return option_desc == NULL ? NULL : option_desc->description;
}

option_type_t GetOptionType(const char * const name)
{
  const option_description_t * const option_desc = FindOptionDescription(name);
  return option_desc == NULL ? OT_INVALID : option_desc->type;
}

int GetAllowedOptionValueRange(const char * const name, int * const restricted, size_t * const min, size_t * const max)
{
  const option_description_t * const option_desc = FindOptionDescription(name);
  if (name == NULL)
    return ERROR_INVALID_VALUE;
  switch (option_desc->type)
  {
    case OT_BOOL:
    case OT_FLOAT:
    case OT_CHAR:
      *restricted = 0;
      break;
    case OT_SIZE:
      *restricted = 1;
      *min = option_desc->min_value;
      *max = option_desc->max_value;
      break;
    case OT_INVALID:
    default:
      return ERROR_INVALID_VALUE;
  }
  return NO_ERROR;
}

void SetDefaultOptions(options_t * const options)
{
  options->adaptive = 0; /* Don't use adaptive coding */
  options->block_size_bits = 8; /* Use one-byte blocks */
  options->column = 1; /* Use first columns */
  options->normalization_factor = 100; /* Two decimal places */
  options->num_decimal_places = 2; /* Two decimal places */
  options->separator_char = ','; /* Comma-separated values */
  options->value_size_bits = 32; /* 32-bit values (TODO: reduce when MAX_USABLE_SIZE is < 1<<32 - 1) */
  options->num_values = 2; /* Sum up two consecutive values */
}

static int EncoderSupportsOptionInternal(const named_enc_dec_t * const encoder, const option_t option)
{
  return encoder == NULL ? 0 : ((encoder->supported_options & option) == option);
}

static int EncoderSupportsOptionNameInternal(const named_enc_dec_t * const encoder, const char * const option_name)
{
  const option_description_t * const option_desc = FindOptionDescription(option_name);
  return option_desc == NULL ? 0 : EncoderSupportsOptionInternal(encoder, option_desc->option);
}

int EncoderSupportsOption(const char * const encoder_name, const char * const option_name)
{
  const named_enc_dec_t * const encoder = FindEncoder(encoder_name);
  return encoder == NULL ? 0 : EncoderSupportsOptionNameInternal(encoder, option_name);
}

int EncoderFromFunctionSupportsOption(enc_dec_function_t * const function, int encoder, const char * const option_name)
{
  size_t i;
  for (i = 0; i < num_encoders; i++)
  {
    if ((encoder && encoders_decoders[i].encoder_decoder.encoder == function) || (!encoder && encoders_decoders[i].encoder_decoder.decoder == function))
      return EncoderSupportsOptionNameInternal(&encoders_decoders[i], option_name);
  }
  return 0;
}

#define GET_OPTION_VALUE(T) \
  { \
    const option_description_t * const option_desc = FindOptionDescription(name); \
    if (option_desc == NULL) \
      return ERROR_INVALID_VALUE; \
    *value = *(const T*)((const uint8_t*)options + option_desc->struct_offset); \
    return NO_ERROR; \
  }

int GetOptionValueBool(const options_t * const options, const char * const name, int * const value)
{
  GET_OPTION_VALUE(int);
}

int GetOptionValueSize(const options_t * const options, const char * const name, size_t * const value)
{
  GET_OPTION_VALUE(size_t);
}

int GetOptionValueFloat(const options_t * const options, const char * const name, float * const value)
{
  GET_OPTION_VALUE(float);
}

int GetOptionValueChar(const options_t * const options, const char * const name, char * const value)
{
  GET_OPTION_VALUE(char);
}

#define SET_OPTION_VALUE(T) \
  { \
    const option_description_t * const option_desc = FindOptionDescription(name); \
    if (option_desc == NULL) \
      return ERROR_INVALID_VALUE; \
    *(T*)((uint8_t*)options + option_desc->struct_offset) = value; \
    return NO_ERROR; \
  }

int SetOptionValueBool(options_t * const options, const char * const name, const int value)
{
  SET_OPTION_VALUE(int);
}

int SetOptionValueSize(options_t * const options, const char * const name, const size_t value)
{
  SET_OPTION_VALUE(size_t);
}

int SetOptionValueFloat(options_t * const options, const char * const name, const float value)
{
  SET_OPTION_VALUE(float);
}

int SetOptionValueChar(options_t * const options, const char * const name, const char value)
{
  SET_OPTION_VALUE(char);
}