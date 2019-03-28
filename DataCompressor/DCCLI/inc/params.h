/* CLI parameter parsing and usage printing (header)
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#ifndef _PARAMS_H
#define _PARAMS_H

#include "enc_dec.h"

#define MAX_OPTIONS 16

typedef struct parameters_t
{
  size_t num_options;
  options_t *options[MAX_OPTIONS];
  FILE *error_log_file;
} parameters_t;

int ProcessParameters(const int argc, const char * const * const argv, parameters_t * const parameters, FILE ** const in_file, FILE ** const out_file);

#endif