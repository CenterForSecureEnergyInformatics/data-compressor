MAKE ?= make

MAIN_APP = DCCLI
MAIN_APP_PATH = ../../$(MAIN_APP)
MAIN_APP_BUILD_PATH = $(MAIN_APP_PATH)/build/gcc

all: $(MAIN_APP)

debug: SUBTARGET = debugall
debug: $(MAIN_APP)

release: SUBTARGET = releaseall
release: $(MAIN_APP)

test: SUBTARGET = test
test: $(MAIN_APP)

clean: SUBTARGET = cleanall
clean: $(MAIN_APP)

$(MAIN_APP): FORCE
	$(MAKE) -C $(MAIN_APP_BUILD_PATH) $(SUBTARGET)

FORCE: