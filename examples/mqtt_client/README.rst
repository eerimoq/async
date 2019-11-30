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
   CC /home/erik/workspace/async/src/async_utils_linux.c
   CC /home/erik/workspace/async/src/async.c
   CC /home/erik/workspace/async/src/async_timer.c
   CC /home/erik/workspace/async/src/async_channel.c
   CC /home/erik/workspace/async/src/async_shell.c
   CC /home/erik/workspace/async/src/async_mqtt_client.c
   CC /home/erik/workspace/async/src/ports/linux/async_core.c
   CC /home/erik/workspace/async/src/ports/linux/async_tcp_client.c
   CC /home/erik/workspace/async/3pp/bitstream/bitstream.c
   CC /home/erik/workspace/async/3pp/humanfriendly/src/hf.c
   CC main.c
   LD /home/erik/workspace/async/examples/mqtt_client/build/app
   Starting the MQTT client.
   Connected.
   Starting the publish timer with timeout 1000 ms.
   Publishing 'count: 0' on 'async/hello'.
   Publishing 'count: 1' on 'async/hello'.
   Publishing 'count: 2' on 'async/hello'.
   Publishing 'count: 3' on 'async/hello'.
   Publishing 'count: 4' on 'async/hello'.
   Publishing 'count: 5' on 'async/hello'.
   Stopping the publish timer.
   Starting the publish timer with timeout 2000 ms.
   Publishing 'count: 6' on 'async/hello'.
   Publishing 'count: 7' on 'async/hello'.
   Publishing 'count: 8' on 'async/hello'.
   Publishing 'count: 9' on 'async/hello'.
   Stopping the MQTT client for 3000 ms.
   Stopping the publish timer.
   Starting the MQTT client.
   Connected.
   Starting the publish timer with timeout 2000 ms.
   Publishing 'count: 10' on 'async/hello'.
   Publishing 'count: 11' on 'async/hello'.
   Publishing 'count: 12' on 'async/hello'.

.. _MQTT broker: https://github.com/eerimoq/mqttools
