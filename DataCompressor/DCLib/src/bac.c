/* Binary arithmetic coder
   Part of DataCompressor
   Andreas Unterweger, 2015 */

/* This code is based on the listings from
   Witten, I. H., Neal, R. M. and Cleary, J. G.: Arithmetic Coding for Data Compression. Communications of the ACM vol. 30, no. 6, pp. 521-540, June 1987. */

#include "err_codes.h"
#include "io_macros.h"
#include "bac.h"

/* Number of bits (precision) to represent the coding range [0,1] */
#define RANGE_BITS 16

/* For intermediate range size calculations, we need a type which is at least twice as large as the type which specifies the ranges */
#if 2 * RANGE_BITS > IO_SIZE_BITS
  #error "RANGE_BITS must be at most half as large as IO_SIZE_BITS"
#endif

typedef UINT_TYPE(RANGE_BITS) range_t;

#define MAX_RANGE UINT_MAXVALUE(RANGE_BITS)
#define QUARTER_RANGE_BORDER (MAX_RANGE / 4 + 1)
#define HALF_RANGE_BORDER (2 * QUARTER_RANGE_BORDER)
#define THREE_QUARTERS_RANGE_BORDER (3 * QUARTER_RANGE_BORDER)

#define MAX_FREQUENCY (MAX_RANGE >> 2) /* Max. symbol frequency (precision) is 2 bits smaller than the total range */

#define NUMBER_OF_SYMBOLS 2 /* Binary arithmetic coding (2 symbols) */
#define EOF_SYMBOL_INDEX (NUMBER_OF_SYMBOLS + 1)
#define TOTAL_NUMBER_OF_SYMBOLS (NUMBER_OF_SYMBOLS + 1) /* Number of symbols including EOF symbol */

static size_t symbol_to_index[NUMBER_OF_SYMBOLS];
static int index_to_symbol[TOTAL_NUMBER_OF_SYMBOLS + 1];

static range_t symbol_frequencies[TOTAL_NUMBER_OF_SYMBOLS + 1];
static range_t cumulative_symbol_frequencies[TOTAL_NUMBER_OF_SYMBOLS + 1];

static void InitModel(void)
{
  size_t i;
  for (i = 0; i < NUMBER_OF_SYMBOLS; i++)
  {
    symbol_to_index[i] = i + 1;
    index_to_symbol[i + 1] = (int)i;
  }
  for (i = 0; i <= TOTAL_NUMBER_OF_SYMBOLS; i++) /* Assume equal probability/frequencies for all symbols */
  {
    symbol_frequencies[i] = i == 0 ? 0 : 1;
    cumulative_symbol_frequencies[i] = (range_t)(TOTAL_NUMBER_OF_SYMBOLS - i);
  }
}

static void UpdateModel(size_t last_symbol_index)
{
  size_t i;
  if (cumulative_symbol_frequencies[0] == MAX_FREQUENCY) /* Halve all frequencies when their sum exceeds the maximum value */
  {
    range_t cumulative_frequency = 0;
    i = TOTAL_NUMBER_OF_SYMBOLS + 1;
    while (i-- != 0)
    {
      symbol_frequencies[i] = (symbol_frequencies[i] + 1) / 2; /* Round towards +inf */
      cumulative_symbol_frequencies[i] = cumulative_frequency;
      cumulative_frequency += symbol_frequencies[i];
    }
  }
  for (i = last_symbol_index; symbol_frequencies[i] == symbol_frequencies[i - 1]; i--); /* Find position (index) of last symbol */
  if (i < last_symbol_index) /* Update symbol position (due to its new, higher probability (see below)) */
  {
    int current_symbol = index_to_symbol[i];
    int last_symbol = index_to_symbol[last_symbol_index];
    index_to_symbol[i] = last_symbol;
    index_to_symbol[last_symbol_index] = current_symbol;
    symbol_to_index[current_symbol] = last_symbol_index;
    symbol_to_index[last_symbol] = i;
  }
  symbol_frequencies[i]++;
  while (i-- > 0) /* Update cumulative symbol frequencies */
    cumulative_symbol_frequencies[i]++;
}

static range_t start_range, end_range;
static size_t next_bits;

static void StartEncoding(void)
{
  start_range = 0;
  end_range = MAX_RANGE;
  next_bits = 0;
}

static io_int_t OutputNextBits(int current_bit, bit_file_buffer_t * const out_bit_buf, FILE * const error_log_file)
{
  const io_uint_t current_bit_pattern = current_bit ? ~0 : 0;
  WRITE_VALUE_BITS_CHECKED(&current_bit_pattern, (size_t)1, out_bit_buf, error_log_file);
  while (next_bits != 0)
  {
    const io_uint_t inverse_current_bit_pattern = ~current_bit_pattern;
    const size_t bits_to_write = next_bits > IO_SIZE_BITS ? IO_SIZE_BITS : next_bits;
    WRITE_VALUE_BITS_CHECKED(&inverse_current_bit_pattern, bits_to_write, out_bit_buf, error_log_file);
    next_bits -= bits_to_write;
  }
  return NO_ERROR;
}

static io_int_t EncodeSymbol(size_t input_symbol_index, bit_file_buffer_t * const out_bit_buf, FILE * const error_log_file)
{
  const io_uint_t range = (io_uint_t)(end_range - start_range) + 1;
  end_range = start_range + (range_t)((range * cumulative_symbol_frequencies[input_symbol_index - 1]) / cumulative_symbol_frequencies[0]) - 1;
  start_range += (range_t)((range * cumulative_symbol_frequencies[input_symbol_index]) / cumulative_symbol_frequencies[0]);
  for (;;) /* Renormalize */
  {
    io_int_t ret;
    if (end_range < HALF_RANGE_BORDER)
    {
      if ((ret = OutputNextBits(0, out_bit_buf, error_log_file)) != NO_ERROR)
        return ret;
    }
    else if (start_range >= HALF_RANGE_BORDER)
    {
      if ((ret = OutputNextBits(1, out_bit_buf, error_log_file)) != NO_ERROR)
        return ret;
      start_range -= HALF_RANGE_BORDER;
      end_range -= HALF_RANGE_BORDER;
    }
    else if (start_range >= QUARTER_RANGE_BORDER && end_range < THREE_QUARTERS_RANGE_BORDER)
    {
      next_bits++;
      start_range -= QUARTER_RANGE_BORDER;
      end_range -= QUARTER_RANGE_BORDER;
    }
    else
      break;
    start_range *= 2;
    end_range = 2 * end_range + 1;
  }
  return NO_ERROR;
}

static io_int_t FinishEncoding(bit_file_buffer_t * const out_bit_buf, FILE * const error_log_file)
{
  next_bits++;
  return OutputNextBits(start_range < QUARTER_RANGE_BORDER ? 0 : 1, out_bit_buf, error_log_file); /* Output two bits which specify the current quarter of the range */
}

io_int_t EncodeBAC(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  io_int_t ret;
  InitModel();
  StartEncoding();
  while (!EndOfBitFileBuffer(in_bit_buf))
  {
    io_uint_t input_bit;
    size_t input_symbol_index;
    READ_VALUE_BITS_CHECKED(&input_bit, (size_t)1, in_bit_buf, options->error_log_file);
    input_symbol_index = symbol_to_index[input_bit];
    if ((ret = EncodeSymbol(input_symbol_index, out_bit_buf, options->error_log_file)) != NO_ERROR)
      return ret;
    if (options->adaptive)
      UpdateModel(input_symbol_index);
  }
  if ((ret = EncodeSymbol(EOF_SYMBOL_INDEX, out_bit_buf, options->error_log_file)) != NO_ERROR) /* Terminate with EOF symbol */
    return ret;
  return FinishEncoding(out_bit_buf, options->error_log_file);
}

static range_t current_value;
static size_t after_eof_bits;

static io_int_t ReadBitSpecial(bit_file_buffer_t * const in_bit_buf, io_uint_t * const input_bit, FILE * const error_log_file) /* Read bit and allow for up to after_eof_bits bits of "garbage" after EOF */
{
  if (EndOfBitFileBuffer(in_bit_buf))
  {
    if (after_eof_bits > 0)
    {
      after_eof_bits--;
      *input_bit = 0; /* Garbage bit after EOF */
      return NO_ERROR;
    }
    else /* Read too many bits after EOF */
      return ERROR_INVALID_FORMAT;
  }
  READ_VALUE_BITS_CHECKED(input_bit, (size_t)1, in_bit_buf, error_log_file);
  return NO_ERROR;
}

static io_int_t StartDecoding(bit_file_buffer_t * const in_bit_buf, FILE * const error_log_file)
{
  size_t i;
  current_value = 0;
  after_eof_bits = RANGE_BITS - 2;
  for (i = 1; i <= RANGE_BITS; i++) /* Fill range variable initially */
  {
    io_uint_t input_bit;
    io_int_t ret;
    if ((ret = ReadBitSpecial(in_bit_buf, &input_bit, error_log_file)) != NO_ERROR)
      return ret;
    current_value = 2 * current_value + (range_t)input_bit;
  }
  start_range = 0;
  end_range = MAX_RANGE;
  return NO_ERROR;
}

static io_int_t DecodeSymbol(bit_file_buffer_t * const in_bit_buf, size_t * const decoded_symbol_index, FILE * const error_log_file)
{
  const io_uint_t range = (io_uint_t)(end_range - start_range) + 1;
  const range_t current_cumulative_frequency = (range_t)((((io_uint_t)(current_value - start_range) + 1) * cumulative_symbol_frequencies[0] - 1) / range);
  for (*decoded_symbol_index = 1; cumulative_symbol_frequencies[*decoded_symbol_index] > current_cumulative_frequency; (*decoded_symbol_index)++);
  end_range = start_range + (range_t)((range * cumulative_symbol_frequencies[*decoded_symbol_index - 1]) / cumulative_symbol_frequencies[0]) - 1;
  start_range += (range_t)((range * cumulative_symbol_frequencies[*decoded_symbol_index]) / cumulative_symbol_frequencies[0]);
  for (;;) /* Renormalize */
  {
    io_uint_t input_bit;
    io_int_t ret;
    if (end_range < HALF_RANGE_BORDER)
    {
      /* Don't do anything */
    }
    else if (start_range >= HALF_RANGE_BORDER)
    {
      current_value -= HALF_RANGE_BORDER;
      start_range -= HALF_RANGE_BORDER;
      end_range -= HALF_RANGE_BORDER;
    }
    else if (start_range >= QUARTER_RANGE_BORDER && end_range < THREE_QUARTERS_RANGE_BORDER)
    {
      current_value -= QUARTER_RANGE_BORDER;
      start_range -= QUARTER_RANGE_BORDER;
      end_range -= QUARTER_RANGE_BORDER;
    }
    else
      break;
    start_range *= 2;
    end_range = 2 * end_range + 1;
    if ((ret = ReadBitSpecial(in_bit_buf, &input_bit, error_log_file)) != NO_ERROR)
      return ret;
    current_value = 2 * current_value + (range_t)input_bit;
  }
  return NO_ERROR;
}

io_int_t DecodeBAC(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  io_int_t ret;
  InitModel();
  if ((ret = StartDecoding(in_bit_buf, options->error_log_file)) != NO_ERROR)
    return ret;
  for (;;)
  {
    io_uint_t output_bit;
    size_t decoded_symbol_index;
    if ((ret = DecodeSymbol(in_bit_buf, &decoded_symbol_index, options->error_log_file)) != NO_ERROR)
      return ret;
    if (decoded_symbol_index == EOF_SYMBOL_INDEX) /* Terminate on EOF symbol */
      break;
    output_bit = index_to_symbol[decoded_symbol_index];
    WRITE_VALUE_BITS_CHECKED(&output_bit, (size_t)1, out_bit_buf, options->error_log_file);
    if (options->adaptive)
      UpdateModel(decoded_symbol_index);
  }
  return NO_ERROR;
}