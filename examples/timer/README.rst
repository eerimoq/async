About
=====

Showcase periodic and single shot timers.

Compile and run
===============

.. code-block:: text

   $ make -s
   CC /home/erik/workspace/async/src/async_linux.c
   CC main.c
   CC /home/erik/workspace/async/src/async_core.c
   CC /home/erik/workspace/async/src/async_message.c
   CC /home/erik/workspace/async/src/async_timer.c
   CC /home/erik/workspace/async/src/async_queue.c
   CC /home/erik/workspace/async/src/async_mqtt_client.c
   LD /home/erik/workspace/async/examples/timer/build/app
   Starting timer 1.
   Starting timer 2.
   Starting timer 3.
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
   Timer 2 expired.
