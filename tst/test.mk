INC += $(ASYNC_ROOT)/3pp/bitstream/include
INC += $(ASYNC_ROOT)/3pp/humanfriendly/include
INC += $(ASYNC_ROOT)/3pp/monolinux-c-library/include
INC += $(ASYNC_ROOT)/3pp/mbedtls/include
INC += $(ASYNC_ROOT)/3pp/mbedtls/crypto/include

SRC += $(ASYNC_ROOT)/3pp/bitstream/src/bitstream.c

SRC += $(ASYNC_ROOT)/3pp/humanfriendly/src/hf.c

SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_libc.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_message.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_queue.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_timer.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_worker_pool.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_log_object.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_bus.c

SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/aes.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/aesni.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/arc4.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/aria.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/asn1parse.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/asn1write.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/base64.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/bignum.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/blowfish.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/camellia.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ccm.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/certs.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/chacha20.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/chachapoly.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/cipher.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/cipher_wrap.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/cmac.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ctr_drbg.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/debug.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/des.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/dhm.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ecdh.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ecdsa.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ecjpake.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ecp.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ecp_curves.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/entropy.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/entropy_poll.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/error.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/gcm.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/havege.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/hkdf.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/hmac_drbg.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/md2.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/md4.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/md5.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/md.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/md_wrap.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/memory_buffer_alloc.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/net_sockets.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/nist_kw.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/oid.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/padlock.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/pem.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/pk.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/pkcs11.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/pkcs12.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/pkcs5.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/pkparse.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/pk_wrap.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/pkwrite.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/platform.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/platform_util.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/poly1305.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ripemd160.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/rsa.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/rsa_internal.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/sha1.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/sha256.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/sha512.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ssl_cache.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ssl_ciphersuites.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ssl_cli.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ssl_cookie.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ssl_srv.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ssl_ticket.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/ssl_tls.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/threading.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/timing.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/version.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/version_features.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/x509.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/x509_create.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/x509_crl.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/x509_crt.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/x509_csr.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/x509write_crt.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/x509write_csr.c
SRC += $(ASYNC_ROOT)/3pp/mbedtls/library/xtea.c

INC += $(ASYNC_ROOT)/include

SRC += $(ASYNC_ROOT)/src/core/async_core.c
SRC += $(ASYNC_ROOT)/src/core/async_timer.c
SRC += $(ASYNC_ROOT)/src/core/async_channel.c
SRC += $(ASYNC_ROOT)/src/core/async_tcp_client.c
SRC += $(ASYNC_ROOT)/src/core/async_tcp_server.c
SRC += $(ASYNC_ROOT)/src/core/async_runtime_null.c
SRC += $(ASYNC_ROOT)/src/modules/async_stcp_client.c
SRC += $(ASYNC_ROOT)/src/modules/async_stcp_server.c
SRC += $(ASYNC_ROOT)/src/modules/async_ssl.c
SRC += $(ASYNC_ROOT)/src/modules/async_shell.c
SRC += $(ASYNC_ROOT)/src/modules/async_mqtt_client.c
SRC += $(ASYNC_ROOT)/src/runtimes/async_runtime.c
SRC += $(ASYNC_ROOT)/src/runtimes/async_runtime_linux.c
SRC += $(ASYNC_ROOT)/src/utils/async_utils_linux.c

INC += $(ASYNC_ROOT)/tst/utils

CFLAGS += -D_GNU_SOURCE=1
CFLAGS += -ffunction-sections -fdata-sections

SRC += $(ASYNC_ROOT)/tst/utils/utils.c
SRC += $(ASYNC_ROOT)/tst/utils/runtime_test.c
SRC += $(ASYNC_ROOT)/tst/utils/runtime_test_impl.c
SRC += $(ASYNC_ROOT)/tst/utils/subprocess.c

NALA ?= nala
BUILD = build
EXE = $(BUILD)/app
INC += $(BUILD)
INC += $(CURDIR)
INC += $(shell $(NALA) include_dir)
SRC += $(BUILD)/nala_mocks.c
SRC += $(shell $(NALA) c_sources)
SRC += $(TESTS)
# To evaluate once for fewer nala include_dir/c_sources calls.
INC := $(INC)
SRC := $(SRC)
OBJ = $(patsubst %,$(BUILD)%,$(abspath $(SRC:%.c=%.o)))
OBJDEPS = $(OBJ:%.o=%.d)
MOCKGENDEPS = $(BUILD)/nala_mocks.ldflags.d
DEPS = $(OBJDEPS) $(MOCKGENDEPS)
CFLAGS += $(INC:%=-I%)
CFLAGS += -g
CFLAGS += -O0
CFLAGS += -no-pie
CFLAGS += -coverage
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -Wno-unused-command-line-argument
CFLAGS += -std=gnu11
ifeq ($(SANITIZE), yes)
CFLAGS += -fsanitize=address
CFLAGS += -fsanitize=undefined
endif
CFLAGS += -DNALA_INCLUDE_NALA_MOCKS_H
MOCKGENFLAGS += $(IMPLEMENTATION:%=-i %)
MOCKGENFLAGS += $(NO_IMPLEMENTATION:%=-n %)
REPORT_JSON = $(BUILD)/report.json
EXEARGS += $(ARGS)
EXEARGS += $(JOBS:%=-j %)
EXEARGS += $(REPORT_JSON:%=-r %)
LIBS ?= pthread
LSAN_OPTIONS = \
	suppressions=$(ASYNC_ROOT)/make/lsan-suppressions.txt \
	print_suppressions=0

.PHONY: all build generate clean coverage gdb gdb-run auto auto-run help

all: build
	LSAN_OPTIONS="$(LSAN_OPTIONS)" $(EXE) $(EXEARGS)

auto:
	$(MAKE) || true
	while true ; do \
	    $(MAKE) auto-run ; \
	done

auto-run:
	for f in $(OBJDEPS) ; do \
	    ls -1 $$(cat $$f | sed s/\\\\//g | sed s/.*://g) ; \
	done | sort | uniq | grep -v $(BUILD) | entr -d -p $(MAKE)

build: generate
	$(MAKE) $(EXE)

generate: $(BUILD)/nala_mocks.ldflags

clean:
	rm -rf $(BUILD)

coverage:
	gcovr $(GCOVR_ARGS) --html-details --output index.html $(BUILD)
	mkdir -p $(BUILD)/coverage
	mv index.* $(BUILD)/coverage
	@echo "Code coverage report: $$(readlink -f $(BUILD)/coverage/index.html)"

# Recursive make for helpful output.
gdb:
	test_file_func=$$($(EXE) --print-test-file-func $(TEST)) && \
	$(MAKE) gdb-run TEST_FILE_FUNC=$$test_file_func

gdb-run:
	gdb \
	    -ex "b $(TEST_FILE_FUNC)_before_fork" \
	    -ex "r $(TEST)" \
	    -ex "set follow-fork-mode child" \
	    -ex c \
	    $(EXE)

help:
	@echo "TARGET        DESCRIPTION"
	@echo "---------------------------------------------------------"
	@echo "all           Build and run with given ARGS."
	@echo "auto          Build and run with given ARGS on source change."
	@echo "clean         Remove build output."
	@echo "coverage      Create the code coverage report."
	@echo "gdb           Debug given test TEST with gdb."

$(EXE): $(OBJ)
	echo "LD $@"
	$(CC) $(CFLAGS) @$(BUILD)/nala_mocks.ldflags $^ $(LIBS:%=-l%) -o $@

define COMPILE_template
$(patsubst %.c,$(BUILD)%.o,$(abspath $1)): $1 $(BUILD)/nala_mocks.ldflags
	@echo "CC $1"
	mkdir -p $$(@D)
	$$(CC) -MMD $$(CFLAGS) -c -o $$@ $$<
	$(NALA) wrap_internal_symbols $(BUILD)/nala_mocks.ldflags $$@
endef
$(foreach file,$(SRC),$(eval $(call COMPILE_template,$(file))))

$(BUILD)/nala_mocks.ldflags: $(TESTS)
	echo "MOCKGEN $(TESTS)"
	mkdir -p $(@D)
	[ -f $(BUILD)/nala_mocks.h ] || touch $(BUILD)/nala_mocks.h
	$(NALA) cat $(TESTS) \
	    | $(CC) $(CFLAGS) -DNALA_GENERATE_MOCKS -x c -E - \
	    | $(NALA) generate_mocks $(MOCKGENFLAGS) -o $(BUILD)
	cat $(TESTS) \
	    | $(CC) -MM -MT $@ $(CFLAGS) -x c -o $@.d -
	touch $@

-include $(DEPS)
