COMMONMAKDIR = ../../../common/build/gcc
COMMONMAK = $(COMMONMAKDIR)/common.mak
include $(COMMONMAK)

SRCDIR = ../../src
OBJDIR = ../../build/gcc
DESTDIR = ../../../build/gcc

INCDIR = ../../inc
COMMONINCDIR = ../../../common/inc
INC = -I$(INCDIR) -I$(COMMONINCDIR)

EXTRA_LIB = DCLib
EXTRA_LIB_PATH = ../../../$(EXTRA_LIB)
EXTRA_LIB_BUILD_PATH = $(EXTRA_LIB_PATH)/build/gcc
EXTRA_LIB_DIR = $(DESTDIR)
EXTRA_LIB_FILE = $(EXTRA_LIB_DIR)/$(EXTRA_LIB).a
EXTRA_LIB_INC_DIR = $(EXTRA_LIB_PATH)/inc
INC += -I$(EXTRA_LIB_INC_DIR)

EXTRA_LIB_DEP = DCIOLib
EXTRA_LIB_DEP_PATH = ../../../$(EXTRA_LIB_DEP)
EXTRA_LIB_DEP_INC_DIR = $(EXTRA_LIB_DEP_PATH)/inc
INC += -I$(EXTRA_LIB_DEP_INC_DIR)

LDFLAGS += -L$(EXTRA_LIB_DIR) $(EXTRA_LIB_FILE)

EXENAME = DCCLI
EXEFILE = $(DESTDIR)/$(EXENAME)

TESTDATA_PATH = ../../testdata
TESTDATA_INPUT = $(TESTDATA_PATH)/input.txt
TESTDATA_OUTPUT = $(TESTDATA_PATH)/output.txt

CFLAGS += $(INC)

SRC = $(wildcard $(SRCDIR)/*.c)
HDR = $(wildcard $(INCDIR)/*.h) $(wildcard $(COMMONINCDIR)/*.h)
OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRC:.c=.o)))

all: release

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: LDFLAGS += $(DEBUG_LDFLAGS)
debug: $(EXEFILE)

debugall: SUBTARGET = debugall
debugall: debug

release: CFLAGS += $(RELEASE_CFLAGS)
release: $(EXEFILE)

releaseall: SUBTARGET = releaseall
releaseall: release

$(EXEFILE): $(OBJ) $(EXTRA_LIB_FILE)
	$(CC) $(filter-out $(EXTRA_LIB_FILE), $^) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HDR)
	$(CC) $(CFLAGS) $< -o $@

$(EXTRA_LIB_FILE): FORCE
	$(MAKE) -C $(EXTRA_LIB_BUILD_PATH) $(SUBTARGET)

clean:
	$(RM) $(OBJ) $(EXEFILE)

cleanall: clean cleansub

cleansub:
	$(MAKE) -C $(EXTRA_LIB_BUILD_PATH) cleanall

FORCE:

test: $(EXEFILE)
	$(EXEFILE) $(TESTDATA_INPUT) $(TESTDATA_OUTPUT) decode csv \# encode normalize \# encode diff \# encode seg \# encode bac adaptive \# decode bac adaptive \# decode seg \# decode diff \# decode normalize \# encode csv
	$(DIFF) "$(TESTDATA_INPUT)" "$(TESTDATA_OUTPUT)"