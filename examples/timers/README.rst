About
=====

Showcase periodic and single shot timers.

Compile and run
===============

.. code-block:: text

   $ make -s
   CC timers.c
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
   LD /home/erik/workspace/async/examples/timers/build/app
   Timer 1 expired.
   Timer 1 expired.
   Timer 1 expired.
   Timer 2 expired.
   Timer 1 expired.
   Timer 1 expired.
   Timer 3 expired. Stopping timer 1.
   Timer 2 expired.
   Timer 2 expired.
   Timer 2 expired.
   Timer 2 expired.
   Timer 2 expired.
