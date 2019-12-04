all: test
	$(MAKE) -C examples/timers build
	$(MAKE) -C examples/conversation build
	$(MAKE) -C examples/shell build
	$(MAKE) -C examples/mqtt_client build
	$(MAKE) -C examples/tcp_echo_client build
	$(MAKE) -C examples/hello_world build
	$(MAKE) -C examples/counter build

test:
	$(MAKE) -C tst/core/async run
	$(MAKE) -C tst/core/timer run
	$(MAKE) -C tst/core/channel run
	$(MAKE) -C tst/core/tcp_client run
	$(MAKE) -C tst/modules/shell run
	$(MAKE) -C tst/modules/mqtt_client run

clean:
	$(MAKE) -C examples/timers clean
	$(MAKE) -C examples/conversation clean
	$(MAKE) -C examples/shell clean
	$(MAKE) -C examples/mqtt_client clean
	$(MAKE) -C examples/tcp_echo_client clean
	$(MAKE) -C examples/hello_world clean
	$(MAKE) -C examples/counter clean
	$(MAKE) -C tst/core/async clean
	$(MAKE) -C tst/core/timer clean
	$(MAKE) -C tst/core/channel clean
	$(MAKE) -C tst/core/tcp_client clean
	$(MAKE) -C tst/modules/shell clean
	$(MAKE) -C tst/modules/mqtt_client clean
