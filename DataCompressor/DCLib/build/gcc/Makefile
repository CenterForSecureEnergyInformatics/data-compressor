COMMONMAKDIR = ../../../common/build/gcc
COMMONMAK = $(COMMONMAKDIR)/common.mak
include $(COMMONMAK)

SRCDIR = ../../src
OBJDIR = ../../build/gcc
DESTDIR = ../../../build/gcc

INCDIR = ../../inc
COMMONINCDIR = ../../../common/inc
INC = -I$(INCDIR) -I$(COMMONINCDIR)

EXTRA_LIB = DCIOLib
EXTRA_LIB_PATH = ../../../$(EXTRA_LIB)
EXTRA_LIB_BUILD_PATH = $(EXTRA_LIB_PATH)/build/gcc
EXTRA_LIB_DIR = $(DESTDIR)
EXTRA_LIB_FILE = $(EXTRA_LIB_DIR)/$(EXTRA_LIB).a
EXTRA_LIB_INC_DIR = $(EXTRA_LIB_PATH)/inc
INC += -I$(EXTRA_LIB_INC_DIR)

LIBNAME = DCLib
AFILE = $(DESTDIR)/$(LIBNAME).a

CFLAGS += $(INC)

SRC = $(wildcard $(SRCDIR)/*.c)
HDR = $(wildcard $(INCDIR)/*.h) $(wildcard $(COMMONINCDIR)/*.h)
OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRC:.c=.o)))

all: release

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(AFILE)

debugall: SUBTARGET = debug
debugall: debug

release: CFLAGS += $(RELEASE_CFLAGS)
release: $(AFILE)

releaseall: SUBTARGET = release
releaseall: release

$(AFILE): $(OBJ) $(EXTRA_LIB_FILE)
	$(AR) rcs $@ $(filter-out $(EXTRA_LIB_FILE), $^)
	$(MKDIR) temp && \
	$(CD) temp && \
	$(AR) x ../$(EXTRA_LIB_FILE) && \
	$(CD) .. && \
	$(AR) rs $@ temp/*.o && \
	$(RM) temp/* && \
	$(RMDIR) temp

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HDR)
	$(CC) $(CFLAGS) $< -o $@

$(EXTRA_LIB_FILE): FORCE
	$(MAKE) -C $(EXTRA_LIB_BUILD_PATH) $(SUBTARGET)

clean:
	$(RM) $(OBJ) $(AFILE)

cleanall: clean cleansub

cleansub:
	$(MAKE) -C $(EXTRA_LIB_BUILD_PATH) clean

FORCE: