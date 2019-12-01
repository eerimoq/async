BUILD = $(shell readlink -f build)
EXE = $(BUILD)/suite
CFLAGS += -fno-omit-frame-pointer
# CFLAGS += -fsanitize=address
CFLAGS += -fsanitize=undefined
CFLAGS += -coverage
CFLAGS += -Wall -Wextra -std=gnu11
CFLAGS += -g -Og
CFLAGS += -DUNIT_TEST
CFLAGS += -no-pie
LDFLAGS_MOCKS = $(shell cat $(BUILD)/nala_mocks.ld)
COVERAGE_FILTERS +=
INC += $(ASYNC_ROOT)/tst/utils
INC += $(BUILD)
SRC += $(ASYNC_ROOT)/tst/utils/nala.c
SRC += $(BUILD)/nala_mocks.c
NALA = nala

.PHONY: all run build coverage

all: run
	$(MAKE) coverage

build: $(BUILD)/nala_mocks.h
	$(MAKE) $(EXE)

run: build
	$(EXE)

$(BUILD)/nala_mocks.h: main.c
	mkdir -p $(BUILD)
	[ -f nala_mocks.h ] || touch $(BUILD)/nala_mocks.h
	$(CC) $(INC:%=-I%) -D_GNU_SOURCE=1 -E main.c \
	    | $(NALA) generate_mocks -o $(BUILD)

coverage:
	gcovr --root ../.. \
	    --exclude-directories ".*tst.*" $(COVERAGE_FILTERS:%=-f %) \
	    --html-details --output index.html build
	mkdir -p $(BUILD)/coverage
	mv index.* $(BUILD)/coverage
	@echo
	@echo "Code coverage report: $$(readlink -f build/coverage/index.html)"
	@echo

include $(ASYNC_ROOT)/make/common.mk
