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
   Got message 'count: 0' on topic 'async/hello'.
   Publishing 'count: 1'.
   Got message 'count: 1' on topic 'async/hello'.
   Publishing 'count: 2'.
   Got message 'count: 2' on topic 'async/hello'.
   Publishing 'count: 3'.
   Got message 'count: 3' on topic 'async/hello'.
   Publishing 'count: 4'.
   Got message 'count: 4' on topic 'async/hello'.
   Publishing 'count: 5'.
   Got message 'count: 5' on topic 'async/hello'.

.. _MQTT broker: https://github.com/eerimoq/mqttools
