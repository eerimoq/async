About
=====

Showcase the MQTT client.

Compile and run
===============

First of all, start an `MQTT broker`_ in another terminal.

.. code-block:: text

   $ mqttools -l debug broker

Then build and run the example program.

In another terminal, publish on ``async/start`` to start the publish
timer, ``async/stop`` to stop the publish timer and
``async/reconnect`` to reconnect to the broker.

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
   Starting the MQTT client.
   Connected.
   Starting the publish timer with timeout 1000 ms.
   Publishing 'count: 0'.
   Publishing 'count: 1'.
   Publishing 'count: 2'.
   Publishing 'count: 3'.
   Publishing 'count: 4'.
   Publishing 'count: 5'.
   Stopping the publish timer.
   Starting the publish timer with timeout 2000 ms.
   Publishing 'count: 6'.
   Publishing 'count: 7'.
   Publishing 'count: 8'.
   Publishing 'count: 9'.
   Stopping the MQTT client for 3000 ms.
   Stopping the publish timer.
   Starting the MQTT client.
   Connected.
   Starting the publish timer with timeout 2000 ms.
   Publishing 'count: 10'.
   Publishing 'count: 11'.
   Publishing 'count: 12'.

.. _MQTT broker: https://github.com/eerimoq/mqttools
