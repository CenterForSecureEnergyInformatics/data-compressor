/* LZMH encoder/decoder
   (Third-party-code-based) Part of DataCompressor
   Martin Ringwelski, 2011
   Andreas Unterweger, 2015 */

/* This code is an implementation of the LZMH algorithm described in
   Ringwelski, M., Renner, C., Reinhardt, A., Weigel, A. and Turau, V.: The Hitchhiker's guide to choosing the compression algorithm for your smart meter data". In 2012 IEEE International Energy Conference and Exhibition (ENERGYCON), pp.935-940, September 2012
   The code was provided by Martin Ringwelski. It has been modified so that it can be compiled with MSVC and gcc */

#include "err_codes.h"
#include "io_macros.h"
#include "lzmh.h"

/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.
  
   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see http://www.gnu.org/licenses/.
*/

/*
* CompressLZMH.c
*
*  This Code is under the GNU Lesser General Public License.
*
* Binary Codes:
*      - 0b00 + Bytecode               uncompressed Byte
*      - 0b010 + Offset + Length       LZ Coding
*      - 0b0110 + Length               Last offset
*      - 0b01110 + Length              Second last offset
*      - 0b011110 + Length             Third last offset
*      - 0b011111 + Length             Fourth last offset
*      - 0b1 + Huffman Code            Huffman encoded Data
*
* Length Encoding:
*      - 0b0 + 3 bit                   Encoding length 3 to 10
*      - 0b10 + 3 bit                  Encoding length 11 to 18
*      - 0b11 + 8 bit                  Encoding length 19 to 274
*
*  Created on: 28.09.2011
*      Author: Martin Ringwelski
*  Modified on: 04.05.2015
*      Author: Andreas Unterweger
*/

#define LZ_MAX_OFFSET	128 /* More than 128 breaks the code */

#define LZ_MAX_LENGTH	274	/* More than 274 breaks the code */

#define HUFF_BYTE_LENGTH 8 /* Length of a byte in bit. As we are only
   using ASCII Codes 7 bit are enough. */

#define HUFF_LIST_LENGTH 48 /* here should only be 31 symbols, but in
   the future there may be more, so I set
   it to 48. */

/*#define INTERN_BUFFER_LENGTH 160  Before overwriting the input data, the
   compressed data is stored in a buffer.
   Needs to be at least LZ_MAX_OFFSET + 1. */
#define INTERN_BUFFER_LENGTH    (LZ_MAX_OFFSET + LZ_MAX_LENGTH + 1)

#define CODE_BUFFER	32
typedef io_uint_t	code_t;

#define HUFF_SYMBOL_NOT_FOUND	0xFFFF

typedef struct {
  uint8_t		symbol;
  int       	        count;
} HUFFLIST;

typedef struct {
  uint8_t	code;
  uint8_t	length;
} CODE_ELEMENT;

#define TREE_LENGHT	19
static const CODE_ELEMENT tree[TREE_LENGHT] = {
  { 0x0F /*0b1111*/, 4 },
  { 0x0E /*0b1110*/, 4 },
  { 0x0D /*0b1101*/, 4 },
  { 0x0C /*0b1100*/, 4 },
  { 0x17 /*0b10111*/, 5 },
  { 0x16 /*0b10110*/, 5 },
  { 0x15 /*0b10101*/, 5 },
  { 0x14 /*0b10100*/, 5 },
  { 0x13 /*0b10011*/, 5 },
  { 0x25 /*0b100101*/, 6 },
  { 0x24 /*0b100100*/, 6 },
  { 0x23 /*0b100011*/, 6 },
  { 0x22 /*0b100010*/, 6 },
  { 0x43 /*0b1000011*/, 7 },
  { 0x42 /*0b1000010*/, 7 },
  { 0x83 /*0b10000011*/, 8 },
  { 0x82 /*0b10000010*/, 8 },
  { 0x81 /*0b10000001*/, 8 },
  { 0x80 /*0b10000000*/, 8 }
};


#include <stdio.h>

size_t getPosInBuffer(int pos, size_t size) {
  while (pos < 0) {
      pos += (int)size;
  }
  while ((size_t)pos >= size) {
      pos -= (int)size;
  }
  return (size_t) pos;
}

/**
* Compress
* compresses the data in the buffer and writes the result to the same buffer.
*
* @param buffer	pointer to the buffer
* @param insize	size of the input data
*
* @return	size of the compressed data (or negative value on error)
*/
io_int_t EncodeLZMH(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options) {

  int   		maxoffset, offset, bestoffset;
  int   		offsets[4];

  /* The following variables are not only used for the LZMA coding,
     but also for the Huffmancoding. */
  int           	maxlength, length, bestlength;

  static HUFFLIST	list[HUFF_LIST_LENGTH];
  code_t		code_sym = 0;
  int8_t		code_length = 0;

  static uint8_t	internBuffer[INTERN_BUFFER_LENGTH];
  size_t		internBufferWrite = 0;
  size_t		internBufferRead = 0;
  size_t                internBufferHistory = 0;

  uint8_t		i;	/* used for several purposes */

  /* Initialize Symbollist */
  for (i = 0; i < HUFF_LIST_LENGTH; i++) {
    list[(size_t)i].count = 0;
    /* list[i].symbol	= 0; */
  }

  /* Initialize Offsets */
  for (i = 0; i < 4; i++) {
    offsets[(size_t)i] = 0;
  }

  /* Fill Buffer */
  while (!EndOfBitFileBuffer(in_bit_buf) &&
          (internBufferWrite < INTERN_BUFFER_LENGTH))
  {
      READ_VALUE_BITS_CHECKED((io_uint_t * const)&code_sym, 8, in_bit_buf, options->error_log_file);
      internBuffer[internBufferWrite] = (uint8_t)code_sym;
      internBufferWrite++;
  }
  if (internBufferWrite >= INTERN_BUFFER_LENGTH) {
      internBufferWrite -= INTERN_BUFFER_LENGTH;
  }

  while (!EndOfBitFileBuffer(in_bit_buf) || internBufferWrite != internBufferRead) {
    code_sym = 0;
    if (internBufferHistory != 0) {
      maxoffset = LZ_MAX_OFFSET;
    } else {
      maxoffset = (uint8_t)internBufferRead;
    }
    if (internBufferWrite > internBufferRead) {
        maxlength = (uint16_t)((internBufferWrite - internBufferRead) > LZ_MAX_LENGTH ?
            LZ_MAX_LENGTH :
            (internBufferWrite - internBufferRead));
    } else if (internBufferWrite < internBufferRead) {
        maxlength = (uint16_t)((internBufferWrite + INTERN_BUFFER_LENGTH - internBufferRead) > LZ_MAX_LENGTH ?
                    LZ_MAX_LENGTH :
                    (internBufferWrite + INTERN_BUFFER_LENGTH - internBufferRead));
    } else {
        maxlength = LZ_MAX_LENGTH;
    }

    bestlength = 2;
    bestoffset = 0;

    /* Search the hole History */
    for (offset = 1;
      (offset <= maxoffset) &&
      (bestlength < maxlength);
    ++offset) {

      if ((internBuffer[getPosInBuffer((int)internBufferRead - offset, INTERN_BUFFER_LENGTH)] == internBuffer[internBufferRead]) &&
          (internBuffer[getPosInBuffer((int)internBufferRead - offset + bestlength, INTERN_BUFFER_LENGTH)] == internBuffer[getPosInBuffer((int)internBufferRead + bestlength, INTERN_BUFFER_LENGTH)])) {

        for (length = 1;
          (length < maxlength) &&
              (internBuffer[getPosInBuffer((int)internBufferRead - offset + length, INTERN_BUFFER_LENGTH)] == internBuffer[getPosInBuffer((int)internBufferRead + length, INTERN_BUFFER_LENGTH)]);
        ++length);

        if (length > bestlength) {
          bestlength = length;
          bestoffset = offset;
        }
      }
    }

    if (bestlength >= 3) { /* LZMA Coding */

      /* Write Offset */
      if (offsets[0] == bestoffset) {
        code_length += 4;
        code_sym |= 0x06 /*0b0110*/ << (CODE_BUFFER - code_length);
        /*printf("LZ Offset 1, Length: %d \t", bestlength);*/
      }
      else if (offsets[1] == bestoffset) {
        offsets[1] = offsets[0];
        offsets[0] = bestoffset;

        code_length += 5;
        code_sym |= 0x0E /*0b01110*/ << (CODE_BUFFER - code_length);
        /*printf("LZ Offset 2, Length: %d \t", bestlength);*/
      }
      else if (offsets[2] == bestoffset) {
        offsets[2] = offsets[1];
        offsets[1] = offsets[0];
        offsets[0] = bestoffset;

        code_length += 6;
        code_sym |= 0x1E /*0b011110*/ << (CODE_BUFFER - code_length);
        /*printf("LZ Offset 3, Length: %d \t", bestlength);*/
      }
      else if (offsets[3] == bestoffset) {
        offsets[3] = offsets[2];
        offsets[2] = offsets[1];
        offsets[1] = offsets[0];
        offsets[0] = bestoffset;

        code_length += 6;
        code_sym |= 0x1F /*0b011111*/ << (CODE_BUFFER - code_length);
        /*printf("LZ Offset 4, Length: %d \t", bestlength);*/
      }
      else {
        offsets[3] = offsets[2];
        offsets[2] = offsets[1];
        offsets[1] = offsets[0];
        offsets[0] = bestoffset;

        code_length += 10;
        code_sym |= (0x100 /*0b0100000000*/ | (bestoffset - 1)) <<
          (CODE_BUFFER - code_length);
        /*printf("LZ Offset: %d, Length: %d \t", bestoffset, bestlength);*/
      }

      /* Write Length */
      if (bestlength < 11) { /* 3 - 10 */
        code_length += 4;
        code_sym |= (bestlength - 3) <<
          (CODE_BUFFER - code_length);
      }
      else if (bestlength < 19) { /* 11 - 18 */
        code_length += 5;
        code_sym |= (0x10 | (bestlength - 11)) <<
          (CODE_BUFFER - code_length);
      }
      else { /* 19 - LZ_MAX_LENGTH */
        code_length += 10;
        code_sym |= (0x300 | (bestlength - 19)) <<
          (CODE_BUFFER - code_length);
      }

      internBufferRead += bestlength;
      if (internBufferRead >= INTERN_BUFFER_LENGTH) {
          internBufferRead -= INTERN_BUFFER_LENGTH;
      }
    }
    else { /* ATH Coding */
      length = 0;
      maxlength = HUFF_SYMBOL_NOT_FOUND;

      i = internBuffer[internBufferRead++];

      /*printf("ATH: %X\t", i);*/

      if (internBufferRead >= INTERN_BUFFER_LENGTH) {
          internBufferRead -= INTERN_BUFFER_LENGTH;
      }

      /* Search for the Symbol in the list */
      while (length < HUFF_LIST_LENGTH && list[length].count > 0)  {
        if (list[length].symbol == i) {
          maxlength = length;
          if (list[length].count < ((1 << 16) - 1)) {
            bestlength = list[length].count + 1;

            /* Bubble Sort */
            while ((length > 0) && (bestlength > (int)list[length - 1].count)) {
              list[length].symbol = list[length - 1].symbol;
              length--;
            }

            list[length].count = bestlength;
            list[length].symbol = i;
          }
          break;
        }
        length++;
      }

      if (maxlength == HUFF_SYMBOL_NOT_FOUND && length < HUFF_LIST_LENGTH) {
        /* Add Symbol to list */
        list[length].symbol = i;
        list[length].count = 1;
      }

      if (maxlength < TREE_LENGHT) { /* Huffman Code */
        code_length += tree[maxlength].length;
        code_sym |= tree[maxlength].code << (
          CODE_BUFFER - code_length);
      }
      else { /* Bytecode */
        code_length += HUFF_BYTE_LENGTH + 2;
        code_sym |= i << (CODE_BUFFER -
          code_length);
      }

    }

    code_sym = code_sym >> (CODE_BUFFER - code_length);
    /*printf("%X, %d\n", (unsigned int)code_sym, code_length);*/
    WRITE_VALUE_BITS_CHECKED(&code_sym, code_length, out_bit_buf, options->error_log_file);
    code_length = 0;

    if (internBufferRead > internBufferHistory) {
        if ((internBufferRead - internBufferHistory) > LZ_MAX_OFFSET) {
            internBufferHistory = (internBufferRead - LZ_MAX_OFFSET);
        }
    } else {
        if ((internBufferRead + INTERN_BUFFER_LENGTH - internBufferHistory) > LZ_MAX_OFFSET) {
            internBufferHistory = (internBufferRead + INTERN_BUFFER_LENGTH - LZ_MAX_OFFSET) % INTERN_BUFFER_LENGTH;
        }
    }

    /* Fill Buffer */
    while (!EndOfBitFileBuffer(in_bit_buf) &&
            (internBufferWrite != internBufferHistory))
    {
        READ_VALUE_BITS_CHECKED((io_uint_t * const)&code_sym, 8, in_bit_buf, options->error_log_file);
        internBuffer[internBufferWrite] = (uint8_t)code_sym;
        internBufferWrite++;
        if (internBufferWrite >= INTERN_BUFFER_LENGTH) {
            internBufferWrite -= INTERN_BUFFER_LENGTH;
        }
    }

    /*printf("internBufferRead: %d, internBufferWrite: %d, internBufferHistory: %d\n",
        internBufferRead, internBufferWrite, internBufferHistory);*/

  }

  return NO_ERROR;
}

/**
* DeCompress
* decompresses the data in the inbuffer and writes it to the outbuffer
*
* @param inbuffer	Pointer to the compressed data
* @param insize	Size of the compressed data
* @param outbuffer	Pointer to the output data
* @param outsize	Maximum output size, default 0xFFFF
*
* @return	Size of the decompressed data (or negative value on error)
*/
io_int_t DecodeLZMH(bit_file_buffer_t * const in_bit_buf, bit_file_buffer_t * const out_bit_buf, const options_t * const options)
{
  size_t        i;
  int           length;
  int           offset;
  io_uint_t     tmp;

  int   	offsets[4];

  HUFFLIST	list[HUFF_LIST_LENGTH];
  uint32_t		code_sym = 0;
  int8_t		code_length = 0;

  static uint8_t        internBuffer[LZ_MAX_OFFSET];
  size_t                internBufferPos = 0;

  for (i = 0; i < HUFF_LIST_LENGTH; i++) {
    list[i].count = 0;
    /* list[i].symbol	= 0; */
  }
  for (i = 0; i < 4; i++) {
    offsets[i] = 0;
  }

  /* Main decompression loop */
  do {
    while (!EndOfBitFileBuffer(in_bit_buf) && (CODE_BUFFER - code_length) >= 8) {
      code_length += 1;
      READ_VALUE_BITS_CHECKED((io_uint_t * const)&tmp, 1, in_bit_buf, options->error_log_file);
      code_sym |= tmp << (CODE_BUFFER - code_length);
    }
    if ((code_sym & 0x80000000) != 0) {
      /* Huffman Code */
      /*printf("HuffmanCode\n");*/
      for (i = 0; i < TREE_LENGHT; i++) {
        if ((code_length >= tree[i].length) &&
          (code_sym >> (32 - tree[i].length)) == tree[i].code) {

          code_length -= tree[i].length;
          code_sym = code_sym << tree[i].length;

          tmp = (io_uint_t)list[i].symbol;
          WRITE_VALUE_BITS_CHECKED(&tmp, 8, out_bit_buf, options->error_log_file);
          internBuffer[internBufferPos++] = list[i].symbol;
          if (internBufferPos >= LZ_MAX_OFFSET) {
              internBufferPos -= LZ_MAX_OFFSET;
          }

          if (list[i].count < ((1 << 16) - 1)) {
            length = list[i].count + 1;

            /* Bubble Sort */
            while ((i > 0) && (length > (int)list[i - 1].count)) {
              list[i].symbol = list[i - 1].symbol;
              i--;
            }

            list[i].count = (uint16_t)length;
            list[i].symbol = (uint8_t)tmp;
          }

          break;
        }
      }
      if (i == TREE_LENGHT) {
        return 0;
      }
    }
    else if ((code_sym & 0x40000000) == 0) {
      /* Prefix Code */
      i = (code_sym >> (CODE_BUFFER - (HUFF_BYTE_LENGTH + 2))) & ((1 << HUFF_BYTE_LENGTH) - 1);
      code_length -= (HUFF_BYTE_LENGTH + 2);
      code_sym = code_sym << (HUFF_BYTE_LENGTH + 2);

      tmp = (io_uint_t)i;
      WRITE_VALUE_BITS_CHECKED(&tmp, 8, out_bit_buf, options->error_log_file);
      internBuffer[internBufferPos++] = (uint8_t)i;
      if (internBufferPos >= LZ_MAX_OFFSET) {
          internBufferPos -= LZ_MAX_OFFSET;
      }
      for (length = 0; length < HUFF_LIST_LENGTH && list[length].count > 0 && list[length].symbol != i; length++);
      if (length < HUFF_LIST_LENGTH && list[length].count < ((1 << 16) - 1)) {
        /* Because we don't need an offset here, we use the variable
           for saving the count. */
        offset = list[length].count + 1;

        /* Bubble Sort */
        while ((length > 0) && (offset > (int)list[length - 1].count)) {
          list[length] = list[length - 1];
          length--;
        }

        list[length].count = offset;
        list[length].symbol = (uint8_t)i;
      }
      /*printf("PrefixCode: %X\n", (unsigned int)i);*/
    }
    else {
      /* LZMA Code */
      /*printf("LZMACode ");*/

      code_length -= 2;
      code_sym = code_sym << 2;

      /* Offset */
      if ((code_sym & 0x80000000) == 0) {
        /*printf("enc ");*/
        offset = ((code_sym >> 24) & 0x7F) + 1;
        code_length -= 8;
        code_sym = code_sym << 8;
        offsets[3] = offsets[2];
        offsets[2] = offsets[1];
        offsets[1] = offsets[0];
        offsets[0] = offset;
      }
      else {
        code_length -= 1;
        code_sym = code_sym << 1;
        if ((code_sym & 0x80000000) == 0) {
          /*printf("1 ");*/
          offset = offsets[0];
        }
        else {
          code_length -= 1;
          code_sym = code_sym << 1;
          if ((code_sym & 0x80000000) == 0) {
            /*printf("2 ");*/
            offset = offsets[1];
            offsets[1] = offsets[0];
            offsets[0] = offset;
          }
          else {
            code_length -= 1;
            code_sym = code_sym << 1;
            if ((code_sym & 0x80000000) == 0) {
              /*printf("3 ");*/
              offset = offsets[2];
              offsets[2] = offsets[1];
              offsets[1] = offsets[0];
              offsets[0] = offset;
            }
            else {
              /*printf("4 ");*/
              offset = offsets[3];
              offsets[3] = offsets[2];
              offsets[2] = offsets[1];
              offsets[1] = offsets[0];
              offsets[0] = offset;
            }
          }
        }
        code_length -= 1;
        code_sym = code_sym << 1;
      }
      /*printf("Offset: %d, ", (int)offset);*/

      /* Length */
      if ((code_sym & 0x80000000) == 0) {
        length = ((code_sym >> 28) & 0x07) + 3;
        code_length -= 4;
        code_sym = code_sym << 4;
      }
      else {
        code_length -= 1;
        code_sym = code_sym << 1;
        if ((code_sym & 0x80000000) == 0) {
          length = ((code_sym >> 28) & 0x7) + 11;
          code_length -= 4;
          code_sym = code_sym << 4;
        }
        else {
          length = ((code_sym >> 23) & 0xFF) + 19;
          code_length -= 9;
          code_sym = code_sym << 9;
        }
      }
      /*printf("Length: %d\n", (int)length);*/
      for (i = 0; i < (size_t)length; ++i) {
        tmp = (io_uint_t)internBuffer[getPosInBuffer((int)internBufferPos - offset, LZ_MAX_OFFSET)];
        WRITE_VALUE_BITS_CHECKED(&tmp, 8, out_bit_buf, options->error_log_file);
        internBuffer[internBufferPos] = internBuffer[getPosInBuffer((int)internBufferPos - offset, LZ_MAX_OFFSET)];
        internBufferPos++;
        if (internBufferPos >= LZ_MAX_OFFSET) {
            internBufferPos -= LZ_MAX_OFFSET;
        }
      }
    }
    /*printf("Outpos: %d, Outsize: %d\n", (int)outpos, (int)outsize);*/
  } while (!EndOfBitFileBuffer(in_bit_buf) || code_sym > 0);

  return NO_ERROR;
}
