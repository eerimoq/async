INC += $(ASYNC_ROOT)/include
INC += $(ASYNC_ROOT)/3pp/bitstream
INC += $(ASYNC_ROOT)/3pp/humanfriendly/include
SRC += $(ASYNC_ROOT)/src/async.c
SRC += $(ASYNC_ROOT)/src/async_timer.c
SRC += $(ASYNC_ROOT)/src/async_channel.c
SRC += $(ASYNC_ROOT)/src/async_shell.c
SRC += $(ASYNC_ROOT)/src/async_mqtt_client.c
SRC += $(ASYNC_ROOT)/src/async_runtime.c
SRC += $(ASYNC_ROOT)/src/async_runtime_null.c
SRC += $(ASYNC_ROOT)/src/async_runtime_linux.c
SRC += $(ASYNC_ROOT)/src/async_tcp_client.c
SRC += $(ASYNC_ROOT)/src/async_utils_linux.c
SRC += $(ASYNC_ROOT)/3pp/bitstream/bitstream.c
SRC += $(ASYNC_ROOT)/3pp/humanfriendly/src/hf.c
SRC += main.c
OBJ = $(patsubst %,$(BUILD)%,$(abspath $(SRC:%.c=%.o)))
CFLAGS += $(INC:%=-I%)
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -D_GNU_SOURCE=1
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
