About
=====

Showcase the MQTT client.

Compile and run
===============

First of all, start an `MQTT broker`_ in another terminal.

.. code-block:: text

   $ mqttools -l debug broker

Then build and run the example program.

.. code-block:: text

   $ make -s
   CC main.c
   CC /home/erik/workspace/async/src/async.c
   CC /home/erik/workspace/async/src/async_timer.c
   CC /home/erik/workspace/async/src/asyncio_core.c
   CC /home/erik/workspace/async/src/asyncio_tcp.c
   CC /home/erik/workspace/async/src/asyncio_mqtt_client.c
   CC /home/erik/workspace/async/src/bitstream.c
   LD /home/erik/workspace/async/examples/mqtt_client/build/app
   TCP connected.
   Connected.
   Publishing 'count: 0'.
   Publishing 'count: 1'.
   Publishing 'count: 2'.
   Publishing 'count: 3'.
   Publishing 'count: 4'.
   Publishing 'count: 5'.
   Publishing 'count: 6'.

.. _MQTT broker: https://github.com/eerimoq/mqttools
