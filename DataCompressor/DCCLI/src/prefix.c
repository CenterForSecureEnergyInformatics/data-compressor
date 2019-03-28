/* Prefix formatting utilities
   Part of DataCompressor
   Andreas Unterweger, 2015 */

#include "prefix.h"

#include <float.h>

#define SI_PREFIX_STEP 1000

static const char positive_si_prefixes[] = { 'k', 'M', 'G', 'T', 'P', 'E' };
static const int num_positive_prefixes = sizeof(positive_si_prefixes) / sizeof(positive_si_prefixes[0]); /* int instead of size_t for simplicity (the array is smaller than UINT8_MAX anyway) */
static const char negative_si_prefixes[] = { 'm', 'u', 'n', 'p', 'f', 'a' }; /* Use u instead of µ due to potential character set limitations */
static const int num_negative_prefixes = sizeof(negative_si_prefixes) / sizeof(negative_si_prefixes[0]); /* int instead of size_t for simplicity (the array is smaller than UINT8_MAX anyway) */

#define BYTE_PREFIX_STEP 1024

static const char * const byte_prefixes[] = { "ki", "Mi", "Gi", "Ti", "Pi", "Ei" };
static const int num_byte_prefixes = sizeof(byte_prefixes) / sizeof(byte_prefixes[0]); /* int instead of size_t for simplicity (the array is smaller than UINT8_MAX anyway) */

void FormatFloatSI(const float value, char * const str)
{
  float abs_value = value < 0 ? -value : value;
  if ((abs_value >= 1 && abs_value < SI_PREFIX_STEP) || abs_value < FLT_EPSILON)
  {
    sprintf(str, "%.3f ", value);
    return;
  }
  else if (abs_value >= 1)
  {
    int i = -1;
    abs_value /= SI_PREFIX_STEP;
    while (++i < num_positive_prefixes)
    {
      if (abs_value < SI_PREFIX_STEP)
        break;
      abs_value /= SI_PREFIX_STEP;
    }
    if (i != num_positive_prefixes)
      sprintf(str, "%.3f %c", value < 0 ? -abs_value : abs_value, positive_si_prefixes[i]);
    else
      sprintf(str, "%.3f %c", value < 0 ? -abs_value : abs_value * SI_PREFIX_STEP, positive_si_prefixes[num_positive_prefixes - 1]);
  }
  else if (abs_value <= 1)
  {
    int i = -1;
    abs_value *= SI_PREFIX_STEP;
    while (++i < num_negative_prefixes)
    {
      if (abs_value >= 1)
        break;
      abs_value *= SI_PREFIX_STEP;
    }
    if (i != num_negative_prefixes)
      sprintf(str, "%.3f %c", value < 0 ? -abs_value : abs_value, negative_si_prefixes[i]);
    else
      sprintf(str, "%.3f %c", value < 0 ? -abs_value : abs_value / SI_PREFIX_STEP, negative_si_prefixes[num_negative_prefixes - 1]);
  }
  else
    sprintf(str, "NaN");
}

void FormatByte(const io_uint_t value, char * const str)
{
  float fvalue = (float)value;
  int i = -1;
  if (value < BYTE_PREFIX_STEP)
  {
    sprintf(str, "%" IO_UINT_FORMAT " ", value);
    return;
  }
  fvalue /= BYTE_PREFIX_STEP;
  while (i++ < num_byte_prefixes)
  {
    if (fvalue < BYTE_PREFIX_STEP)
      break;
    fvalue /= BYTE_PREFIX_STEP;
  }
  if (i != num_byte_prefixes)
    sprintf(str, "%.3f %s", fvalue, byte_prefixes[i]);
  else
    sprintf(str, "%.3f %s", fvalue * BYTE_PREFIX_STEP, byte_prefixes[num_byte_prefixes - 1]);
}
