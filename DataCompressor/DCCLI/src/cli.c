/* DataCompressor command line interface
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#include "err_codes.h"
#include "log.h"
#include "params.h"
#include "cli.h"

/* Prints execution time and memory consumption */
#define LOG_DIAGNOSTICS

#ifdef LOG_DIAGNOSTICS
  #define LOG_DIAG(f, ...) LOG((f), __VA_ARGS__)
#else
  #define LOG_DIAG(f, ...) {}
#endif

#if defined(LOG_DIAGNOSTICS) || defined(_DEBUG)
  #define LOG_DIAG_OR_DEBUG(f, ...) LOG((f), __VA_ARGS__)
#else
  #define LOG_DIAG_OR_DEBUG(f, ...) {}
#endif

#ifdef LOG_DIAGNOSTICS
  #include "prefix.h"
  #include <time.h>
  #include <float.h>
#endif

static const size_t READ_BUFFER_SIZE = 1024; /* 1 KiB */
static const size_t WRITE_BUFFER_SIZE = 1024; /* 1 KiB */
static const size_t TEMP_BUFFER_SIZE = 2 * 1024; /* 2 KiB */

void InitBufferEnvironment(buffer_environment_t * const buffer_env)
{
  buffer_env->in_file = buffer_env->out_file = NULL;
  buffer_env->in_buf = buffer_env->out_buf = buffer_env->temp_read_buf = buffer_env->temp_write_buf = NULL;
  buffer_env->in_bit_buf = buffer_env->out_bit_buf = buffer_env->temp_read_bit_buf = buffer_env->temp_write_bit_buf = NULL;
}

void UninitBufferEnvironment(buffer_environment_t * const buffer_env)
{
  if (buffer_env->out_file != NULL && buffer_env->out_file != stdout)
    fclose(buffer_env->out_file);
  if (buffer_env->in_file != NULL && buffer_env->in_file != stdin)
    fclose(buffer_env->in_file);
}

static void FreeBuffers(buffer_environment_t * const buffers)
{
  if (buffers->temp_write_bit_buf != NULL)
    FreeBitFileBuffer(buffers->temp_write_bit_buf);
  if (buffers->temp_read_bit_buf != NULL)
    FreeBitFileBuffer(buffers->temp_read_bit_buf);
  if (buffers->out_bit_buf != NULL)
    FreeBitFileBuffer(buffers->out_bit_buf);
  if (buffers->in_bit_buf != NULL)
    FreeBitFileBuffer(buffers->in_bit_buf);
  if (buffers->temp_write_buf != NULL)
    FreeFileBuffer(buffers->temp_write_buf);
  if (buffers->temp_read_buf != NULL)
    FreeFileBuffer(buffers->temp_read_buf);
  if (buffers->out_buf != NULL)
    FreeFileBuffer(buffers->out_buf);
  if (buffers->in_buf != NULL)
    FreeFileBuffer(buffers->in_buf);
}

static int AllocateBuffers(buffer_environment_t * const buffers, FILE * const error_log_file, const int use_temp_buffers)
{
  if ((buffers->in_buf = AllocateFileBuffer()) == NULL)
  {
    LOG_ERROR(error_log_file, "Error allocating input buffer\n");
    FreeBuffers(buffers);
    return ERROR_MEMORY;
  }
  if ((buffers->out_buf = AllocateFileBuffer()) == NULL)
  {
    LOG_ERROR(error_log_file, "Error allocating output buffer\n");
    FreeBuffers(buffers);
    return ERROR_MEMORY;
  }
  if (use_temp_buffers)
  {
    if ((buffers->temp_read_buf = AllocateFileBuffer()) == NULL)
    {
      LOG_ERROR(error_log_file, "Error allocating temporary reading buffer\n");
      FreeBuffers(buffers);
      return ERROR_MEMORY;
    }
    if ((buffers->temp_write_buf = AllocateFileBuffer()) == NULL)
    {
      LOG_ERROR(error_log_file, "Error allocating temporary writing buffer\n");
      FreeBuffers(buffers);
      return ERROR_MEMORY;
    }
  }
  if ((buffers->in_bit_buf = AllocateBitFileBuffer()) == NULL)
  {
    LOG_ERROR(error_log_file, "Error allocating input bit buffer\n");
    FreeBuffers(buffers);
    return ERROR_MEMORY;
  }
  if ((buffers->out_bit_buf = AllocateBitFileBuffer()) == NULL)
  {
    LOG_ERROR(error_log_file, "Error allocating output bit buffer\n");
    FreeBuffers(buffers);
    return ERROR_MEMORY;
  }
  if (use_temp_buffers)
  {
    if ((buffers->temp_read_bit_buf = AllocateBitFileBuffer()) == NULL)
    {
      LOG_ERROR(error_log_file, "Error allocating temporary reading bit buffer\n");
      FreeBuffers(buffers);
      return ERROR_MEMORY;
    }
    if ((buffers->temp_write_bit_buf = AllocateBitFileBuffer()) == NULL)
    {
      LOG_ERROR(error_log_file, "Error allocating temporary writing bit buffer\n");
      FreeBuffers(buffers);
      return ERROR_MEMORY;
    }
  }
  return NO_ERROR;
}

void UninitBuffers(buffer_environment_t * const buffers)
{
  if (buffers->temp_write_bit_buf != NULL)
    UninitBitFileBuffer(buffers->temp_write_bit_buf);
  if (buffers->temp_read_bit_buf != NULL)
    UninitBitFileBuffer(buffers->temp_read_bit_buf);
  if (buffers->out_bit_buf != NULL)
    UninitBitFileBuffer(buffers->out_bit_buf);
  if (buffers->in_bit_buf != NULL)
    UninitBitFileBuffer(buffers->in_bit_buf);
  if (buffers->temp_write_buf != NULL)
    UninitFileBuffer(buffers->temp_write_buf);
  if (buffers->temp_read_buf != NULL)
    UninitFileBuffer(buffers->temp_read_buf);
  if (buffers->out_buf != NULL)
    UninitFileBuffer(buffers->out_buf);
  if (buffers->in_buf != NULL)
    UninitFileBuffer(buffers->in_buf);
  FreeBuffers(buffers);
}

int InitBuffers(buffer_environment_t * const buffers, FILE * const error_log_file, const int use_temp_buffers)
{
  int ret;
  if ((ret = AllocateBuffers(buffers, error_log_file, use_temp_buffers)) != NO_ERROR)
    return ret;
  if ((ret = InitFileBuffer(buffers->in_buf, buffers->in_file, FBM_READING, READ_BUFFER_SIZE)) != NO_ERROR)
  {
    LOG_ERROR(error_log_file, "%s while initializing input file buffer\n", ERROR_MESSAGE_STRING(ret));
    UninitBuffers(buffers);
    return ERROR_LIBRARY_INIT;
  }
  if ((ret = InitFileBuffer(buffers->out_buf, buffers->out_file, FBM_WRITING, WRITE_BUFFER_SIZE)) != NO_ERROR)
  {
    LOG_ERROR(error_log_file, "%s while initializing output file buffer\n", ERROR_MESSAGE_STRING(ret));
    UninitBuffers(buffers);
    return ERROR_LIBRARY_INIT;
  }
  if (use_temp_buffers)
  {
    if ((ret = InitFileBufferInMemory(buffers->temp_read_buf, FBM_READING, TEMP_BUFFER_SIZE)) != NO_ERROR)
    {
      LOG_ERROR(error_log_file, "%s while initializing temporary reading buffer\n", ERROR_MESSAGE_STRING(ret));
      UninitBuffers(buffers);
      return ERROR_LIBRARY_INIT;
    }
    if ((ret = InitFileBufferInMemory(buffers->temp_write_buf, FBM_WRITING, TEMP_BUFFER_SIZE)) != NO_ERROR)
    {
      LOG_ERROR(error_log_file, "%s while initializing temporary writing buffer\n", ERROR_MESSAGE_STRING(ret));
      UninitBuffers(buffers);
      return ERROR_LIBRARY_INIT;
    }
  }
  InitBitFileBuffer(buffers->in_bit_buf, buffers->in_buf);
  InitBitFileBuffer(buffers->out_bit_buf, buffers->out_buf);
  if (use_temp_buffers)
  {
    InitBitFileBuffer(buffers->temp_read_bit_buf, buffers->temp_read_buf);
    InitBitFileBuffer(buffers->temp_write_bit_buf, buffers->temp_write_buf);
  }
  return NO_ERROR;
}

static void SwapTempFileBuffers(buffer_environment_t * const buffers)
{
  file_buffer_t *temp = buffers->temp_read_buf;
  buffers->temp_read_buf = buffers->temp_write_buf;
  buffers->temp_write_buf = temp;
}

static void SwapTempBitBuffers(buffer_environment_t * const buffers)
{
  bit_file_buffer_t *temp = buffers->temp_read_bit_buf;
  buffers->temp_read_bit_buf = buffers->temp_write_bit_buf;
  buffers->temp_write_bit_buf = temp;
}

static void SwapTempBuffers(buffer_environment_t * const buffers)
{
  SwapTempFileBuffers(buffers); /* Swap original file buffer pointers as well so that they point to the correct buffers */
  SwapTempBitBuffers(buffers);
}

static int SwitchTempBuffers(buffer_environment_t * const buffers, FILE * const error_log_file)
{
  int ret;
  if ((ret = SetBitFileBufferMode(buffers->temp_write_bit_buf, FBM_READING)) != NO_ERROR)
  {
    LOG_ERROR(error_log_file, "Could not switch mode of temporary buffer\n");
    return ret;
  }
  SwapTempBuffers(buffers); /* Old data can now be read from the reading buffer; the writing buffer has to be reset */
  ResetBitFileBuffer(buffers->temp_write_bit_buf, FBM_WRITING);
  return NO_ERROR;
}

#if defined(LOG_DIAGNOSTICS) || defined(_DEBUG)
static const char *GetEncoderDecoderName(enc_dec_function_t * const function)
{
  const char * const enc_name = GetEncoderNameFromFunction(function, 1);
  const char * const dec_name = GetEncoderNameFromFunction(function, 0);
  if (enc_name)
    return enc_name;
  else if (dec_name)
    return dec_name;
  else
    return NULL;
}
#endif

#ifdef _DEBUG
static int PrintOption(const char * const option_name, FILE * const output, FILE * const error_log_file, const options_t * const options)
{
  int ret;
  float float_value;
  size_t size_value;
  int bool_value;
  char char_value;
  LOG_DEBUG(output, "%s=", option_name);
  switch (GetOptionType(option_name))
  {
    case OT_SIZE:
      if ((ret = GetOptionValueSize(options, option_name, &size_value)) != NO_ERROR)
      {
        LOG(error_log_file, "Error getting value of option '%s'\n", option_name);
        return ret;
      }
      LOG_DEBUG(output, "%" SIZE_T_FORMAT, SIZE_T_CAST(size_value));
      break;
    case OT_FLOAT:
      if ((ret = GetOptionValueFloat(options, option_name, &float_value)) != NO_ERROR)
      {
        LOG_DEBUG(error_log_file, "Error getting value of option '%s'\n", option_name);
        return ret;
      }
      LOG_DEBUG(output, "%f", float_value);
      break;
    case OT_BOOL:
      if ((ret = GetOptionValueBool(options, option_name, &bool_value)) != NO_ERROR)
      {
        LOG_DEBUG(error_log_file, "Error getting value of option '%s'\n", option_name);
        return ret;
      }
      LOG_DEBUG(output, "%s", bool_value ? "true" : "false");
      break;
    case OT_CHAR:
      if ((ret = GetOptionValueChar(options, option_name, &char_value)) != NO_ERROR)
      {
        LOG_DEBUG(error_log_file, "Error getting value of option '%s'\n", option_name);
        return ret;
      }
      LOG_DEBUG(output, "%c", char_value);
      break;
    case OT_INVALID:
    default:
      break;
  }
  return NO_ERROR;
}

static int PrintEncoderConfiguration(FILE * const output, FILE * const error_log_file, const options_t * const options)
{
  const size_t num_options = GetNumberOfOptions();
  const char **option_names = (const char**)malloc(num_options * sizeof(const char*));
  size_t i;
  if (option_names == NULL)
  {
    LOG_ERROR(error_log_file, "Error allocating %" SIZE_T_FORMAT " bytes while displaying options. This is not supposed to happen\n", SIZE_T_CAST(num_options * sizeof(const char*)));
    return ERROR_MEMORY;
  }
  GetOptionNames(option_names);
  LOG_DEBUG(output, "  Options: ");
  for (i = 0; i < num_options; i++)
  {
    const char * const option_name = option_names[i];
    int ret;
    if (!EncoderFromFunctionSupportsOption(options->encode ? options->encoder_decoder->encoder : options->encoder_decoder->decoder, options->encode, option_name))
      continue;
    if ((ret = PrintOption(option_name, output, error_log_file, options)) != NO_ERROR)
    {
      free((void*)option_names);
      return ret;
    }
    LOG_DEBUG(output, "|"); /* Separate options visually */
  }
  LOG_DEBUG(output, "\n");
  free((void*)option_names);
  return NO_ERROR;
}
#endif

#ifdef LOG_DIAGNOSTICS
static clock_t total_diff = 0;

static float ClockDiffToSeconds(const clock_t diff)
{
  return (float)diff / CLOCKS_PER_SEC;
}

#define FLOAT_TEXT_BUFFER_SIZE (1 /*'-'*/ + (FLT_MAX_10_EXP + 1) /*38+1 digits*/ + 1 /*'.'*/ + FLT_DIG /*Default precision*/ + 1 /*\0*/) /* Adopted from http://stackoverflow.com/questions/7235456/what-are-the-maximum-numbers-of-characters-output-by-sprintf-when-outputting-flo */
static char char_buf[FLOAT_TEXT_BUFFER_SIZE + 10 /* Extra space */];

static void PrintTimeDiffInSecondsSI(const clock_t diff)
{
  const float t = ClockDiffToSeconds(diff);
  FormatFloatSI(t, char_buf);
  LOG_DIAG(stdout, "%ss (%.3f s)", char_buf, t);
}

static void PrintTimeStatistics(const clock_t end, const clock_t start)
{
  const clock_t diff = end - start;
  total_diff += diff;
  LOG_DIAG(stdout, "  Time elapsed: ");
  PrintTimeDiffInSecondsSI(diff);
  LOG_DIAG(stdout, "\n");
}

static void PrintTotalTimeStatistics(void)
{
  LOG_DIAG(stdout, "Total time elapsed: ");
  PrintTimeDiffInSecondsSI(total_diff);
  LOG_DIAG(stdout, "\n");
}

static void PrintByte(const io_uint_t value)
{
  FormatByte(value, char_buf);
  LOG_DIAG(stdout, "%sB", char_buf);
}

static void PrintIOBytes(const io_int_t bytes, const uint8_t bits, const int read)
{
  LOG_DIAG(stdout, "  %s ", read ? "Read" : "Wrote");
  PrintByte(bytes);
  LOG_DIAG(stdout, " (%" IO_INT_FORMAT " bytes) and %" PRIu8 " bits\n", bytes, bits);
}

static void PrintIOStatistics(const bit_file_buffer_t * const read_bit_buf, const bit_file_buffer_t * const write_bit_buf, const int print_read)
{
  io_int_t bytes;
  uint8_t bits;
  if (print_read)
  {
    GetActualBitFileOffset(read_bit_buf, &bytes, &bits);
    PrintIOBytes(bytes, bits, 1);
  }
  GetActualBitFileOffset(write_bit_buf, &bytes, &bits);
  PrintIOBytes(bytes, bits, 0);
}

static void PrintBufferSize(const char * const buffer_name, const size_t buffer_size)
{
  LOG_DIAG(stdout, "    %s buffer size: ", buffer_name);
  PrintByte(buffer_size);
  LOG_DIAG(stdout, " (%" SIZE_T_FORMAT " bytes) \n", SIZE_T_CAST(buffer_size));
}

static void PrintBufferSizes(const buffer_environment_t * buffers)
{
  size_t current_size, total_size = 0;
  LOG_DIAG(stdout, "  Buffer use (without structural overhead):\n")
  total_size += (current_size = GetFileBufferSize(buffers->in_buf));
  PrintBufferSize("Input", current_size);
  total_size += (current_size = GetFileBufferSize(buffers->out_buf));
  PrintBufferSize("Output", current_size);
  if (buffers->temp_read_buf && buffers->temp_write_buf)
  {
    total_size += (current_size = GetFileBufferSize(buffers->temp_read_buf));
    PrintBufferSize("Temporary read", current_size);
    total_size += (current_size = GetFileBufferSize(buffers->temp_write_buf));
    PrintBufferSize("Temporary write", current_size);
  }
  PrintBufferSize("Total", total_size);
}

#endif

int main(const int argc, const char * const * const argv)
{
  buffer_environment_t buffer_env;
  parameters_t parameters;
  int ret;
  size_t i;
  LOG_DEBUG(stdout, BITSIZE_INFO_PRINT_ARGS);
  InitBufferEnvironment(&buffer_env);
  parameters.error_log_file = stderr; /* Default: Log errors to stderr */
  if ((ret = ProcessParameters(argc - 1, &argv[1], &parameters, &buffer_env.in_file, &buffer_env.out_file)) != NO_ERROR)
  {
    UninitBufferEnvironment(&buffer_env);
    return ret;
  }
  if ((ret = InitBuffers(&buffer_env, parameters.error_log_file, parameters.num_options > 1)) != NO_ERROR)
  {
    UninitBuffers(&buffer_env);
    UninitBufferEnvironment(&buffer_env);
    return ret;
  }
#ifdef LOG_DIAGNOSTICS
  PrintBufferSizes(&buffer_env);
#endif
  for (i = 0; i < parameters.num_options; i++)
  {
    options_t * const options = parameters.options[i];
    io_int_t enc_ret;
    bit_file_buffer_t * const read_bit_buf = i == 0 ? buffer_env.in_bit_buf : buffer_env.temp_read_bit_buf; /* First read from input (file), rest from memory */
    bit_file_buffer_t * const write_bit_buf = i == parameters.num_options - 1 ? buffer_env.out_bit_buf : buffer_env.temp_write_bit_buf; /* Last write to output (file), rest to memory */
#ifdef LOG_DIAGNOSTICS
    clock_t start, end;
#endif
    enc_dec_function_t * const enc_dec = options->encode ? options->encoder_decoder->encoder : options->encoder_decoder->decoder;
    LOG_DIAG_OR_DEBUG(stdout, "Executing %s %s (%" SIZE_T_FORMAT " of %" SIZE_T_FORMAT " total)...\n", options->encode ? "encoder" : "decoder", GetEncoderDecoderName(enc_dec), SIZE_T_CAST(i + 1), SIZE_T_CAST(parameters.num_options));
#ifdef _DEBUG
    PrintEncoderConfiguration(stdout, parameters.error_log_file, options);
#endif
#ifdef LOG_DIAGNOSTICS
    start = clock();
#endif
    if ((enc_ret = (*enc_dec)(read_bit_buf, write_bit_buf, options)) != NO_ERROR)
    {
      LOG_ERROR(parameters.error_log_file, "%s while executing encoder/decoder %" SIZE_T_FORMAT " of %" SIZE_T_FORMAT "\n", ERROR_MESSAGE_STRING(enc_ret), SIZE_T_CAST(i + 1), SIZE_T_CAST(parameters.num_options));
      UninitBuffers(&buffer_env);
      UninitBufferEnvironment(&buffer_env);
      return ERROR_LIBRARY_CALL;
    }
#ifdef LOG_DIAGNOSTICS
    end = clock();
    PrintTimeStatistics(end, start);
    PrintIOStatistics(read_bit_buf, write_bit_buf, i == 0);
    PrintBufferSizes(&buffer_env);
#endif
    if (parameters.num_options > 1 && i < parameters.num_options - 1 && (ret = SwitchTempBuffers(&buffer_env, parameters.error_log_file)) != NO_ERROR) /* Switch temporary buffer from reading to reading after all but the last encoding process (only required when the temporary buffer is needed, i.e., with at least two encoders) */
    {
      UninitBuffers(&buffer_env);
      UninitBufferEnvironment(&buffer_env);
      return ERROR_LIBRARY_CALL;
    }
  }
#ifdef LOG_DIAGNOSTICS
  PrintTotalTimeStatistics();
#endif
  UninitBuffers(&buffer_env);
  UninitBufferEnvironment(&buffer_env);
  return 0;
}