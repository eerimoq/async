TEST ?= all

ifneq ($(TEST), all)
TESTS = $(TEST:%=test_%.c)
endif

BUILD = build/$(TEST)
EXE = $(BUILD)/suite
CFLAGS += -fno-omit-frame-pointer
CFLAGS += -fsanitize=address
CFLAGS += -fsanitize=undefined
CFLAGS += -coverage
CFLAGS += -Wall -Wextra -Wpedantic -std=gnu11
CFLAGS += -g -Og
CFLAGS += -DUNIT_TEST
CFLAGS += -no-pie
LDFLAGS_MOCKS = $(shell cat $(BUILD)/nala_mocks.ld)
COVERAGE_FILTERS +=
INC += $(ASYNC_ROOT)/tst/utils
INC += $(BUILD)
SRC += $(ASYNC_ROOT)/tst/utils/utils.c
SRC += $(ASYNC_ROOT)/tst/utils/nala.c
SRC += $(ASYNC_ROOT)/tst/utils/runtime_test.c
SRC += $(ASYNC_ROOT)/tst/utils/runtime_test_impl.c
SRC += $(BUILD)/nala_mocks.c
SRC += $(TESTS)
TESTS ?= main
TESTS_C = $(BUILD)/suites.c

.PHONY: all run build coverage

all: run

build: $(BUILD)/nala_mocks.h
	$(MAKE) $(EXE)

run: build
	$(EXE)

test: run
	$(MAKE) coverage

$(BUILD)/nala_mocks.h: $(TESTS)
	echo "MOCKGEN $^"
	mkdir -p $(BUILD)
	[ -f nala_mocks.h ] || touch $(BUILD)/nala_mocks.h
	cat $(TESTS) > $(TESTS_C)
	$(CC) $(INC:%=-I%) -D_GNU_SOURCE=1 -E $(TESTS_C) \
	    | nala generate_mocks -o $(BUILD)

coverage:
	gcovr --root ../.. \
	    --exclude-directories ".*tst.*" $(COVERAGE_FILTERS:%=-f %) \
	    --html-details --output index.html $(BUILD)
	mkdir -p $(BUILD)/coverage
	mv index.* $(BUILD)/coverage
	@echo
	@echo "Code coverage report: $$(readlink -f $(BUILD)/coverage/index.html)"
	@echo

include $(ASYNC_ROOT)/make/common.mk
