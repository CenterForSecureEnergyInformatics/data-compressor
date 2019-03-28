CC ?= gcc
AR ?= ar
MKDIR ?= mkdir
CD ?= cd
RMDIR ?= rmdir
MAKE ?= make
RM ?= rm -f
DIFF ?= diff

CFLAGS += -c
LDFLAGS += -static

DEBUG_CFLAGS = -D_DEBUG -g -pg
DEBUG_LDFLAGS = -pg
RELEASE_CFLAGS = -O2 -ffast-math

COMMON_CFLAGS = -Wall -Wextra -Werror

#The code is mostly C90-compliant, but with stdint.h and friends (GNU extension), unsigned long long in certain configurations (GNU extension), anonymous variadic macros (officially only in C99 or alternatively as GNU extension with slightly different syntax)
COMMON_CFLAGS += -pedantic -std=gnu90 -Wno-long-long -Wno-variadic-macros
CFLAGS += $(COMMON_CFLAGS)

#Uncomment this if you need to set the number of I/O size bits globally and don't want to set it in the corresponding header file
#IO_SIZE_BITS = 32
#OVERRIDE_IO_SIZE_BITS = -DIO_SIZE_BITS=$(IO_SIZE_BITS)
#CFLAGS += $(OVERRIDE_IO_SIZE_BITS)

#Uncomment this if you need to use 64-bit I/O and the include order of your headers (especially stdio.h) is such that the definitions of io.h are not effective due to prior definitions and includes
#COMMON_EXTRA_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
#CFLAGS += $(COMMON_EXTRA_CFLAGS)
 
