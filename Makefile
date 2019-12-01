test:
	$(MAKE) -C examples/timers build
	$(MAKE) -C examples/conversation build
	$(MAKE) -C examples/shell build
	$(MAKE) -C examples/mqtt_client build
	$(MAKE) -C examples/tcp_echo_client build
	$(MAKE) -C tst/async run
	$(MAKE) -C tst/shell run

clean:
	$(MAKE) -C examples/timers clean
	$(MAKE) -C examples/conversation clean
	$(MAKE) -C examples/shell clean
	$(MAKE) -C examples/mqtt_client clean
	$(MAKE) -C examples/tcp_echo_client clean
	$(MAKE) -C tst/async clean
	$(MAKE) -C tst/shell clean
