test:
	$(MAKE) -C examples/timer build
	#$(MAKE) -C examples/mqtt_client build
	$(MAKE) -C examples/tcp_echo_client build

clean:
	$(MAKE) -C examples/timer build clean
	#$(MAKE) -C examples/mqtt_client clean
	$(MAKE) -C examples/tcp_echo_client clean
