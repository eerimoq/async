About
=====

Showcase periodic and single shot timers.

Compile and run
===============

.. code-block:: text

   $ make -s
   CC timers.c
   CC main.c
   CC /home/erik/workspace/async/src/async.c
   CC /home/erik/workspace/async/src/async_timer.c
   CC /home/erik/workspace/async/src/async_linux.c
   LD /home/erik/workspace/async/examples/async/timers/build/app
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
