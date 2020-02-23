.PHONY: examples library install

BUILD ?= build
LIBRARY = $(BUILD)/libasync.a
PREFIX ?= /usr/local
VERSION = $(shell grep ASYNC_VERSION include/async/core.h | awk '{print $$3}' | tr -d '"')

default: run

all: run examples

run:
	$(MAKE) -C tst run

test:
	$(MAKE) -C tst test

examples:
	$(MAKE) -C examples/timers build
	$(MAKE) -C examples/conversation build
	$(MAKE) -C examples/shell build
	$(MAKE) -C examples/mqtt_client build
	$(MAKE) -C examples/stcp_echo_client build
	$(MAKE) -C examples/hello_world build
	$(MAKE) -C examples/counter build
	$(MAKE) -C examples/http_get build
	$(MAKE) -C examples/call_worker_pool build
	$(MAKE) -C examples/call_threadsafe build

clean:
	rm -rf $(BUILD)
	$(MAKE) -C tst clean
	$(MAKE) -C examples/timers clean
	$(MAKE) -C examples/conversation clean
	$(MAKE) -C examples/shell clean
	$(MAKE) -C examples/mqtt_client clean
	$(MAKE) -C examples/stcp_echo_client clean
	$(MAKE) -C examples/hello_world clean
	$(MAKE) -C examples/counter clean
	$(MAKE) -C examples/http_get clean
	$(MAKE) -C examples/call_worker_pool clean
	$(MAKE) -C examples/call_threadsafe clean

release:
	rm -rf async-core-$(VERSION)
	mkdir async-core-$(VERSION)
	cp -r --parents include/async/core* async-core-$(VERSION)
	cp -r --parents include/async/utils* async-core-$(VERSION)
	cp -r --parents src/core async-core-$(VERSION)
	cp -r --parents src/utils async-core-$(VERSION)
	tar czf async-core-$(VERSION).tar.gz async-core-$(VERSION)

release-compile:
	cd async-core-$(VERSION) && \
	    for f in $$(find -name "*.c") ; do \
		gcc -c -Iinclude $$f ; \
	    done

library: $(LIBRARY)

install:
	find include -type f -exec install -Dm 644 "{}" "$(PREFIX)/{}" \;
	mkdir -p $(PREFIX)/lib
	install -m 644 $(LIBRARY) $(PREFIX)/lib

include make/library.mk

$(LIBRARY): $(OBJ)
	mkdir -p $(BUILD)
	$(AR) cr $(LIBRARY) $^

help:
	@echo "TARGET     DESCRIPTION"
	@echo "------------------------------------------------------"
	@echo "all        run + examples"
	@echo "run        Build and run all tests."
	@echo "test       run + coverage report."
	@echo "examples   Build all examples."
	@echo "clean      Remove build and run files."
	@echo "release    Create a release."
