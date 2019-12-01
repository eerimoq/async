About
=====

Incremet or decrement a counter by pressing ``+`` and ``-``. Prints
the count periodically.

Compile and run
===============

.. code-block:: text

   $ make -s
   CC counter.c
   CC /home/erik/workspace/async/src/async_utils_linux.c
   CC /home/erik/workspace/async/src/async.c
   CC /home/erik/workspace/async/src/async_timer.c
   CC /home/erik/workspace/async/src/async_channel.c
   CC /home/erik/workspace/async/src/async_shell.c
   CC /home/erik/workspace/async/src/async_mqtt_client.c
   CC /home/erik/workspace/async/src/ports/none/async_core.c
   CC /home/erik/workspace/async/src/ports/none/async_tcp_client.c
   CC /home/erik/workspace/async/3pp/bitstream/bitstream.c
   CC /home/erik/workspace/async/3pp/humanfriendly/src/hf.c
   CC main.c
   LD /home/erik/workspace/async/examples/counter/build/app
   Change the count by pressing + and -.
   Count: 0
   Count: 0
   Count: 0
   Count: 1
   Count: 1
   Count: 2
   Count: 1
   Count: -1
   Count: 0
   Count: 2
   Count: 5
   Count: 3
   Count: 3
   Count: 3
