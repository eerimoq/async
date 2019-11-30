INC += $(ASYNC_ROOT)/include
INC += $(ASYNC_ROOT)/3pp/bitstream
INC += $(ASYNC_ROOT)/3pp/humanfriendly/include
ASYNC_SRC ?= \
	$(ASYNC_ROOT)/src/async.c \
	$(ASYNC_ROOT)/src/async_timer.c \
	$(ASYNC_ROOT)/src/async_channel.c \
	$(ASYNC_ROOT)/src/async_shell.c \
	$(ASYNC_ROOT)/src/async_linux.c \
	$(ASYNC_ROOT)/3pp/humanfriendly/src/hf.c
ASYNCIO_SRC ?= \
	$(ASYNC_ROOT)/src/asyncio_core.c \
	$(ASYNC_ROOT)/src/asyncio_tcp_client.c \
	$(ASYNC_ROOT)/src/asyncio_mqtt_client.c \
	$(ASYNC_ROOT)/3pp/bitstream/bitstream.c
SRC += main.c
SRC += $(ASYNC_SRC)
SRC += $(ASYNCIO_SRC)
OBJ = $(patsubst %,$(BUILD)%,$(abspath $(SRC:%.c=%.o)))
CFLAGS += $(INC:%=-I%)
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -D_GNU_SOURCE=1
CFLAGS += -fsanitize=address
LDFLAGS += -Wl,--gc-sections
DEPSDIR = $(BUILD)/deps

.PHONY: clean

app: $(EXE)

$(EXE): $(OBJ)
	@echo "LD $@"
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) $(LDFLAGS_MOCKS) -lpthread -o $@

define COMPILE_template
-include $(patsubst %.c,$(DEPSDIR)%.o.dep,$(abspath $1))
$(patsubst %.c,$(BUILD)%.o,$(abspath $1)): $1
	@echo "CC $1"
	mkdir -p $(BUILD)$(abspath $(dir $1))
	mkdir -p $(DEPSDIR)$(abspath $(dir $1))
	$$(CC) $$(CFLAGS) -c -o $$@ $$<
	gcc -MM -MT $$@ $$(CFLAGS) -o $(patsubst %.c,$(DEPSDIR)%.o.dep,$(abspath $1)) $$<
endef
$(foreach file,$(SRC),$(eval $(call COMPILE_template,$(file))))

clean:
	rm -rf $(BUILD) $(CLEAN)

print-%:
	@echo $($*)
