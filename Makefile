test:
	$(MAKE) -C examples/ping_pong
	$(MAKE) -C examples/timer build
	$(MAKE) -C examples/mqtt_client build

clean:
	$(MAKE) -C examples/ping_pong clean
	$(MAKE) -C examples/timer build clean
	$(MAKE) -C examples/mqtt_client clean
