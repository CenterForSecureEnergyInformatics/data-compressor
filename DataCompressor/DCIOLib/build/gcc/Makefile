COMMONMAKDIR = ../../../common/build/gcc
COMMONMAK = $(COMMONMAKDIR)/common.mak
include $(COMMONMAK)

SRCDIR = ../../src
OBJDIR = ../../build/gcc
DESTDIR = ../../../build/gcc

INCDIR = ../../inc
COMMONINCDIR = ../../../common/inc
INC = -I$(INCDIR) -I$(COMMONINCDIR)

LIBNAME = DCIOLib
AFILE = $(DESTDIR)/$(LIBNAME).a

CFLAGS += $(INC)

SRC = $(wildcard $(SRCDIR)/*.c)
HDR = $(wildcard $(INCDIR)/*.h) $(wildcard $(COMMONINCDIR)/*.h)
OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRC:.c=.o)))

all: release

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(AFILE)

release: CFLAGS += $(RELEASE_CFLAGS)
release: $(AFILE)

$(AFILE): $(OBJ)
	$(AR) rcs $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HDR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJ) $(AFILE)