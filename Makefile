test:
	$(MAKE) -C examples/async/timers build
	$(MAKE) -C examples/async/conversation build
	$(MAKE) -C examples/async/shell build
	$(MAKE) -C examples/asyncio/mqtt_client build
	$(MAKE) -C examples/asyncio/tcp_echo_client build
	$(MAKE) -C tst/async run

clean:
	$(MAKE) -C examples/async/timers clean
	$(MAKE) -C examples/async/conversation clean
	$(MAKE) -C examples/async/shell clean
	$(MAKE) -C examples/asyncio/mqtt_client clean
	$(MAKE) -C examples/asyncio/tcp_echo_client clean
	$(MAKE) -C tst/async clean
