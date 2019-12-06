BUILD = $(shell readlink -f build)
EXE = $(BUILD)/app
CFLAGS += -fno-omit-frame-pointer
CFLAGS += -Wall -Wextra -std=gnu11
CFLAGS += -g -Og
SRC += main.c

.PHONY: all run build

all: run

build: $(EXE)

run: $(EXE)
	$(EXE)

include $(ASYNC_ROOT)/make/common.mk
