BUILD = $(shell readlink -f build)
EXE = $(BUILD)/suite
CFLAGS += -fno-omit-frame-pointer
CFLAGS += -fsanitize=address
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
SRC += utils.c
SRC += $(ASYNC_ROOT)/tst/utils/nala.c
SRC += $(ASYNC_ROOT)/tst/utils/runtime_test.c
SRC += $(ASYNC_ROOT)/tst/utils/runtime_test_impl.c
SRC += $(BUILD)/nala_mocks.c
SRC += $(TESTS)
TESTS ?= main
TESTS_O = $(patsubst %,$(BUILD)%,$(abspath $(TESTS:%.c=%.o)))
NALA = nala

.PHONY: all run build coverage

all: run

build:
	$(MAKE) $(EXE)

run: build
	$(EXE)

test: run
	$(MAKE) coverage

$(TESTS_O): $(BUILD)/nala_mocks.c

$(BUILD)/nala_mocks.c: $(TESTS)
	echo "MOCKGEN $^"
	mkdir -p $(BUILD)
	[ -f nala_mocks.h ] || touch $(BUILD)/nala_mocks.h
	cat $(TESTS) > tests.pp.c
	$(CC) $(INC:%=-I%) -D_GNU_SOURCE=1 -E tests.pp.c \
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
