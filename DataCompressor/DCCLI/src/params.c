/* CLI parameter parsing and usage printing
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#include "err_codes.h"
#include "io.h"
#include "log.h"
#include "params.h"

#include <string.h>

static void PrintUsage(FILE * const error_log_file)
{
  LOG(error_log_file, "DataCompressor CLI (DCCLI)\n");
  LOG(error_log_file, "Usage: <input file> <output file> ('encode'|'decode') <encoder/decoder> [<options>] [# ('encode'|'decode') <encoder/decoder> [<options>] ...]\n");
  LOG(error_log_file, "Examples: input.dat output.dat encode copy\n");
  LOG(error_log_file, "          input.dat output.dat encode copy # decode copy\n");
}

static int PrintEncoders(FILE * const error_log_file)
{
  const size_t num_encoders = GetNumberOfEncoders();
  const char **encoder_names = (const char**)malloc(num_encoders * sizeof(const char*));
  size_t i;
  if (encoder_names == NULL)
  {
    LOG_ERROR(error_log_file, "Error allocating %" SIZE_T_FORMAT " bytes while listing encoders/decoders\n", SIZE_T_CAST(num_encoders * sizeof(const char*)));
    return ERROR_MEMORY;
  }
  GetEncoderNames(encoder_names);
  for (i = 0; i < num_encoders; i++)
    LOG(error_log_file, "  %s: %s\n", encoder_names[i], GetEncoderDescription(encoder_names[i]));
  free((void*)encoder_names);
  return NO_ERROR;
}

static int ParseOptionValueLong(const char * const option_name, const char * argument, io_uint_t * value, FILE * const error_log_file)
{
  char* end_ptr;
  const char * const argument_start = argument + 1; /* Parse over '=' for value */
  if (strlen(argument) == 0 || argument[0] != '=')
  {
    LOG_ERROR(error_log_file, "Expected '=' after option '%s'\n", option_name);
    return ERROR_INVALID_FORMAT;
  }
  if (strlen(argument_start) == 0)
  {
    LOG_ERROR(error_log_file, "Expected value after '=' in option '%s'\n", option_name);
    return ERROR_INVALID_FORMAT;
  }
  *value = IO_STRTOUL(argument_start, &end_ptr, 10);
  if (end_ptr == NULL || end_ptr[0] != '\0')
  {
    LOG_ERROR(error_log_file, "Invalid value '%s' for option '%s'\n", argument_start, option_name);
    return ERROR_INVALID_VALUE;
  }
  return NO_ERROR;
}

static int ParseOptionValueFloat(const char * const option_name, const char * argument, float * value, FILE * const error_log_file)
{
  const char * const argument_start = argument + 1; /* Parse over '=' for value */
  if (strlen(argument) == 0 || argument[0] != '=')
  {
    LOG_ERROR(error_log_file, "Expected '=' after option '%s'\n", option_name);
    return ERROR_INVALID_FORMAT;
  }
  if (strlen(argument_start) == 0)
  {
    LOG_ERROR(error_log_file, "Expected value after '=' in option '%s'\n", option_name);
    return ERROR_INVALID_FORMAT;
  }
  *value = strtof(argument_start, NULL);
  return NO_ERROR;
}

static int ParseOptionValueChar(const char * const option_name, const char * argument, char * value, FILE * const error_log_file)
{
  const char * const argument_start = argument + 1; /* Parse over '=' for value */
  if (strlen(argument) == 0 || argument[0] != '=')
  {
    LOG_ERROR(error_log_file, "Expected '=' after option '%s'\n", option_name);
    return ERROR_INVALID_FORMAT;
  }
  if (strlen(argument_start) == 0)
  {
    LOG_ERROR(error_log_file, "Expected value after '=' in option '%s'\n", option_name);
    return ERROR_INVALID_FORMAT;
  }
  *value = argument_start[0];
  return NO_ERROR;
}

static int PrintOptions(FILE * const error_log_file)
{
  const size_t num_options = GetNumberOfOptions();
  const char **option_names = (const char**)malloc(num_options * sizeof(const char*));
  size_t i;
  if (option_names == NULL)
  {
    LOG_ERROR(error_log_file, "Error allocating %" SIZE_T_FORMAT " bytes while listing options\n", SIZE_T_CAST(num_options * sizeof(const char*)));
    return ERROR_MEMORY;
  }
  GetOptionNames(option_names);
  for (i = 0; i < num_options; i++)
  {
    LOG(error_log_file, "  %s", option_names[i]);
    switch (GetOptionType(option_names[i]))
    {
      case OT_SIZE:
      case OT_FLOAT:
      case OT_CHAR:
        LOG(error_log_file, "=<n>");
        break;
      case OT_BOOL:
      case OT_INVALID:
      default:
        break;
    }
    LOG(error_log_file, ": %s\n", GetOptionDescription(option_names[i]));
  }
  free((void*)option_names);
  return NO_ERROR;
}

static int SetOption(options_t * const options, const char * const current_option_name, const char * const current_option_value)
{
  int ret, restricted;
  io_uint_t temp_value;
  size_t min, max;
  float temp_float_value;
  char temp_char_value;
  switch (GetOptionType(current_option_name))
  {
    case OT_BOOL:
      if (strlen(current_option_value))
      {
        LOG_ERROR(options->error_log_file, "Option '%s' is boolean. Omit the '=' to enable it\n", current_option_name);
        return ERROR_INVALID_FORMAT;
      }
      SetOptionValueBool(options, current_option_name, 1); /* Enable option */
      break;
    case OT_SIZE:
      if ((ret = ParseOptionValueLong(current_option_name, current_option_value, &temp_value, options->error_log_file)) != NO_ERROR)
        return ret;
      if ((ret = GetAllowedOptionValueRange(current_option_name, &restricted, &min, &max)) != NO_ERROR)
        return ret;
      if (restricted && (temp_value < min || temp_value > max))
      {
        LOG_ERROR(options->error_log_file, "The value of option '%s' has to be between %" SIZE_T_FORMAT " and %" SIZE_T_FORMAT "\n", current_option_name, SIZE_T_CAST(min), SIZE_T_CAST(max));
        return ERROR_INVALID_VALUE;
      }
      SetOptionValueSize(options, current_option_name, (size_t)temp_value);
      break;
    case OT_FLOAT:
      if ((ret = ParseOptionValueFloat(current_option_name, current_option_value, &temp_float_value, options->error_log_file)) != NO_ERROR)
        return ret;
      SetOptionValueFloat(options, current_option_name, temp_float_value);
      break;
    case OT_CHAR:
      if ((ret = ParseOptionValueChar(current_option_name, current_option_value, &temp_char_value, options->error_log_file)) != NO_ERROR)
        return ret;
      SetOptionValueChar(options, current_option_name, temp_char_value);
      break;
    case OT_INVALID:
    default:
      break; /* This is not supposed to happen, so just don't do anything and parse the next option */
  }
  return NO_ERROR;
}

static int ParseOptions(const size_t opt_argc, const char * const * const opt_argv, const char * const encoder_name, options_t * const options, size_t * const processed_argc)
{
  int ret;
  size_t i;
  *processed_argc = 0;
  for (i = 0; i < opt_argc; i++)
  {
    const char * const separator_start = strstr(opt_argv[i], "=");
    const char * const current_option_value = separator_start == NULL ? opt_argv[i] + strlen(opt_argv[i]) : separator_start; /* Points to '=' or '\0' of argument if there is no '=' */
    const size_t option_name_length = (current_option_value - opt_argv[i] + 1) / sizeof(char);
    char *current_option_name;
    if (strlen(opt_argv[i]) >= 1 && opt_argv[i][0] == '#') /* End of options (separator) */
    {
      i++; /* Advance to next argument... */
      break; /* ... and abort */
    }
    if ((current_option_name = (char*)malloc(option_name_length * sizeof(char))) == NULL)
    {
      LOG_ERROR(options->error_log_file, "Error allocating %" SIZE_T_FORMAT " bytes while parsing options\n", SIZE_T_CAST(option_name_length * sizeof(char)));
      return ERROR_MEMORY;
    }
    strncpy(current_option_name, opt_argv[i], option_name_length - 1);
    current_option_name[option_name_length - 1] = '\0'; /* Terminate string */
    if (!OptionNameExists(current_option_name))
    {
      LOG_ERROR(options->error_log_file, "Unknown option '%s'. The following options are supported:\n", current_option_name);
      free(current_option_name);
      return (ret = PrintOptions(options->error_log_file)) == NO_ERROR ? ERROR_INVALID_VALUE : ret;
    }
    if (EncoderSupportsOption(encoder_name, current_option_name) == 0)
    {
      LOG(options->error_log_file, "Encoder '%s' does not support option '%s'\n", encoder_name, current_option_name);
      free(current_option_name);
      return ERROR_INVALID_MODE;
    }
    ret = SetOption(options, current_option_name, current_option_value);
    free(current_option_name);
    if (ret != NO_ERROR)
      return ret;
  }
  *processed_argc = i;
  return NO_ERROR;
}

static int ProcessEncoder(const size_t argc, const char * const * const argv, options_t * const options, size_t * const processed_argc)
{
  int encode, decode;
  *processed_argc = 0;
  if (argc < 2) /* At least mode and encoder name are required */
  {
    PrintUsage(options->error_log_file);
    return ERROR_INVALID_FORMAT;
  }
  encode = (strncmp("encode", argv[*processed_argc], strlen("encode")) == 0);
  decode = (strncmp("decode", argv[*processed_argc], strlen("decode")) == 0);
  if (!encode && !decode)
  {
    LOG_ERROR(options->error_log_file, "Unrecognized mode '%s' - only 'encode' and 'decode' are supported\n", argv[*processed_argc]);
    return ERROR_INVALID_VALUE;
  }
  options->encode = encode;
  (*processed_argc)++;
  if ((options->encoder_decoder = GetEncoder(argv[*processed_argc])) == NULL)
  {
    const char * const enc_dec[2] = { "encoder", "decoder" };
    int ret;
    LOG_ERROR(options->error_log_file, "Unknown %s '%s'. The following encoders and decoders are supported:\n", enc_dec[options->encode ? 0 : 1], argv[*processed_argc]);
    return (ret = PrintEncoders(options->error_log_file)) == NO_ERROR ? ERROR_INVALID_VALUE : ret;
  }
  if ((encode && options->encoder_decoder->encoder == NULL) || (decode && options->encoder_decoder->decoder == NULL))
  {
    const char * const enc_dec[2] = { "encoder", "decoder" };
    LOG_ERROR(options->error_log_file, "'%s' is not supported as a %s\n", argv[*processed_argc], enc_dec[options->encode ? 0 : 1]);
    return ERROR_INVALID_MODE;
  }
  (*processed_argc)++;
  SetDefaultOptions(options);
  if (argc > 2)
  {
    int ret;
    size_t additional_processed_argc;
    if ((ret = ParseOptions(argc - *processed_argc, &argv[*processed_argc], argv[1], options, &additional_processed_argc)) != NO_ERROR)
      return ret;
    *processed_argc += additional_processed_argc;
  }
  return NO_ERROR;
}

int ProcessParameters(const int argc, const char * const * const argv, parameters_t * const parameters, FILE ** const in_file, FILE ** const out_file)
{
  size_t current_argc;
  parameters->num_options = 0;
  if (argc < 2) /* At least input/output files are required */
  {
    PrintUsage(parameters->error_log_file);
    return ERROR_INVALID_FORMAT;
  }
  current_argc = 2; /* Skip input and output file for now (check when other parameters are o.k.) */
  do
  {
    int ret;
    size_t processed_argc;
    if (parameters->num_options >= MAX_OPTIONS)
    {
      LOG_ERROR(parameters->error_log_file, "The number of encoders/decoders to be used at once is limited to %d. Reduce the number of encoders/decoders\n", MAX_OPTIONS);
      return ERROR_MEMORY;
    }
    if ((parameters->options[parameters->num_options] = (options_t * const)malloc(sizeof(options_t))) == NULL)
    {
      LOG_ERROR(parameters->error_log_file, "Error allocating %" SIZE_T_FORMAT " bytes while processing parameters\n", SIZE_T_CAST(sizeof(options_t)));
      return ERROR_MEMORY;
    }
    parameters->options[parameters->num_options++]->error_log_file = parameters->error_log_file;
    if ((ret = ProcessEncoder(argc - current_argc, &argv[current_argc], parameters->options[parameters->num_options - 1], &processed_argc)) != NO_ERROR)
      return ret;
    current_argc += processed_argc;
  } while (current_argc < (size_t)argc);
  if (strlen(argv[0]) == 1 && argv[0][0] == '-')
    *in_file = stdin;
  else if ((*in_file = FOPEN(argv[0], "rb")) == NULL)
  {
    LOG_ERROR(parameters->error_log_file, "Could not open input file '%s'\n", argv[0]);
    return ERROR_FILE_IO;
  }
  if (strlen(argv[1]) == 1 && argv[1][0] == '-')
    *out_file = stdout;
  if ((*out_file = FOPEN(argv[1], "wb")) == NULL)
  {
    LOG_ERROR(parameters->error_log_file, "Could not open output file '%s'\n", argv[1]);
    fclose(*in_file);
    *in_file = NULL;
    return ERROR_FILE_IO;
  }
  return NO_ERROR;
}
