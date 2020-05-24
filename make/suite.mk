BUILD = build
EXE = $(BUILD)/suite
CFLAGS += -fno-omit-frame-pointer
ifeq ($(SANITIZE), yes)
CFLAGS += -fsanitize=address
CFLAGS += -fsanitize=undefined
endif
CFLAGS += -coverage
CFLAGS += -Wall -Wextra -std=gnu11
CFLAGS += -g -Og
CFLAGS += -DUNIT_TEST
CFLAGS += -no-pie
CFLAGS += -DNALA_INCLUDE_NALA_MOCKS_H
LDFLAGS_MOCKS = $(shell cat $(BUILD)/nala_mocks.ldflags)
COVERAGE_FILTERS +=
INC += $(ASYNC_ROOT)/tst/utils
INC += $(ASYNC_ROOT)/3pp/nala
INC += $(shell $(NALA) include_dir)
INC += $(BUILD)
SRC += $(ASYNC_ROOT)/tst/utils/utils.c
SRC += $(shell $(NALA) c_sources)
SRC += $(ASYNC_ROOT)/tst/utils/runtime_test.c
SRC += $(ASYNC_ROOT)/tst/utils/runtime_test_impl.c
SRC += $(ASYNC_ROOT)/tst/utils/subprocess.c
SRC += $(BUILD)/nala_mocks.c
SRC += $(TESTS)
TESTS ?= main
TESTS_C = $(BUILD)/suites.c
NALA ?= nala
LSAN_OPTIONS = \
	suppressions=$(ASYNC_ROOT)/make/lsan-suppressions.txt \
	print_suppressions=0

.PHONY: all run build coverage clean

all: run

build:
	$(MAKE) $(BUILD)/nala_mocks.ldflags
	$(MAKE) $(EXE)

run: build
	LSAN_OPTIONS="$(LSAN_OPTIONS)" $(EXE) $(ARGS)

test: run
	$(MAKE) coverage

$(BUILD)/nala_mocks.ldflags: $(TESTS)
	echo "MOCKGEN $^"
	mkdir -p $(BUILD)
	[ -f $(BUILD)/nala_mocks.h ] || touch $(BUILD)/nala_mocks.h
	$(NALA) cat $(TESTS) $(ASYNC_ROOT)/tst/utils/utils.c > $(TESTS_C)
	$(CC) $(INC:%=-I%) -D_GNU_SOURCE=1 -DNALA_GENERATE_MOCKS -E $(TESTS_C) \
	    | $(NALA) generate_mocks -o $(BUILD)
	touch $@

coverage:
	gcovr --root ../.. \
	    --exclude-directories ".*tst.*" $(COVERAGE_FILTERS:%=-f %) \
	    --html-details --output index.html $(BUILD)
	mkdir -p $(BUILD)/coverage
	mv index.* $(BUILD)/coverage
	@echo
	@echo "Code coverage report: $$(readlink -f $(BUILD)/coverage/index.html)"
	@echo

clean:
	rm -rf $(BUILD) $(CLEAN)

include $(ASYNC_ROOT)/make/common.mk
