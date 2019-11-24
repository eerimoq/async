test:
	$(MAKE) -C examples/timer build
	$(MAKE) -C examples/mqtt_client build
	$(MAKE) -C examples/tcp_echo_client build
	$(MAKE) -C examples/conversation build
	$(MAKE) -C tst run

clean:
	$(MAKE) -C examples/timer clean
	$(MAKE) -C examples/mqtt_client clean
	$(MAKE) -C examples/tcp_echo_client clean
	$(MAKE) -C examples/conversation clean
	$(MAKE) -C tst clean
