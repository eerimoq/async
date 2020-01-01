INC += $(ASYNC_ROOT)/include
INC += $(ASYNC_ROOT)/3pp/bitstream
INC += $(ASYNC_ROOT)/3pp/humanfriendly/include
INC += $(ASYNC_ROOT)/3pp/monolinux-c-library/include
INC += $(ASYNC_ROOT)/3pp/mbedtls/include
INC += $(ASYNC_ROOT)/3pp/mbedtls/crypto/include

SRC += $(ASYNC_ROOT)/src/core/async_core.c
SRC += $(ASYNC_ROOT)/src/core/async_timer.c
SRC += $(ASYNC_ROOT)/src/core/async_channel.c
SRC += $(ASYNC_ROOT)/src/core/async_tcp_client.c
SRC += $(ASYNC_ROOT)/src/core/async_runtime_null.c
SRC += $(ASYNC_ROOT)/src/modules/async_stcp_client.c
SRC += $(ASYNC_ROOT)/src/modules/async_ssl.c
SRC += $(ASYNC_ROOT)/src/modules/async_shell.c
SRC += $(ASYNC_ROOT)/src/modules/async_mqtt_client.c
SRC += $(ASYNC_ROOT)/src/runtimes/async_runtime.c
SRC += $(ASYNC_ROOT)/src/runtimes/async_runtime_linux.c
SRC += $(ASYNC_ROOT)/src/utils/async_utils_linux.c

SRC += $(ASYNC_ROOT)/3pp/bitstream/bitstream.c

SRC += $(ASYNC_ROOT)/3pp/humanfriendly/src/hf.c

SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_libc.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_message.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_queue.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_timer.c
SRC += $(ASYNC_ROOT)/3pp/monolinux-c-library/src/ml_worker_pool.c

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
