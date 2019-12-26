About
=====

Call a function that sleeps for 5 seconds in the worker pool. In
parallel a periodic timer is running.

Compile and run
===============

.. code-block:: text

   $ make -s
   ...
   Sleeping for 5 seconds...
   Timeout!
   Timeout!
   Timeout!
   Timeout!
   Timeout!
   Woke up!
   Completed!
   Timeout!
   Timeout!
   Timeout!
