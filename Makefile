.PHONY: examples

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
	$(MAKE) -C examples/hello_world build
	$(MAKE) -C examples/counter build

clean:
	$(MAKE) -C tst clean
	$(MAKE) -C examples/timers clean
	$(MAKE) -C examples/conversation clean
	$(MAKE) -C examples/shell clean
	$(MAKE) -C examples/mqtt_client clean
	$(MAKE) -C examples/tcp_echo_client clean
	$(MAKE) -C examples/hello_world clean
	$(MAKE) -C examples/counter clean
