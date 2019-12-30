.PHONY: examples

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
	$(MAKE) -C examples/tcp_echo_client build
	$(MAKE) -C examples/ssl_tcp_echo_client build
	$(MAKE) -C examples/hello_world build
	$(MAKE) -C examples/counter build
	$(MAKE) -C examples/http_get build
	$(MAKE) -C examples/call_worker_pool build
	$(MAKE) -C examples/call_threadsafe build

clean:
	$(MAKE) -C tst clean
	$(MAKE) -C examples/timers clean
	$(MAKE) -C examples/conversation clean
	$(MAKE) -C examples/shell clean
	$(MAKE) -C examples/mqtt_client clean
	$(MAKE) -C examples/tcp_echo_client clean
	$(MAKE) -C examples/ssl_tcp_echo_client clean
	$(MAKE) -C examples/hello_world clean
	$(MAKE) -C examples/counter clean
	$(MAKE) -C examples/http_get clean
	$(MAKE) -C examples/call_worker_pool clean
	$(MAKE) -C examples/call_threadsafe clean

help:
	@echo "TARGET     DESCRIPTION"
	@echo "------------------------------------------------------"
	@echo "all        run + examples"
	@echo "run        Build and run all tests."
	@echo "test       run + coverage report."
	@echo "examples   Build all examples."
	@echo "clean      Remove build and run files."
