|buildstatus|_
|codecov|_

🔀 Async
=======

Asynchronous framework in C for systems where low memory usage is
important.

See `libuv`_ for a mature library with similar functionality.

Features
========

- Delayed (asynchronous) function calls.

- Timers.

Examples
========

The hello world example, printing 'Hello world!' periodically.

.. code-block:: c

   #include <stdio.h>
   #include "async.h"

   static void on_timeout()
   {
       printf("Hello world!\n");
   }

   int main()
   {
       struct async_t async;
       struct async_timer_t timer;

       async_init(&async);
       async_set_runtime(&async, async_runtime_create());
       async_timer_init(&timer, on_timeout, NULL, ASYNC_TIMER_PERIODIC, &async);
       async_timer_start(&timer, 1000);
       async_run_forever(&async);

       return (0);
   }

There are more examples in the `examples folder`_.

Runtimes
========

A runtime implements zero or more of the following features:

- Tick timers.

- Networking (TCP).

Default
-------

The default runtime does not implement any runtime features. It's
designed for minimal dependencies and easy integration in any
application.

Typical usage:

.. code-block:: c

   async_init(&async);
   ...
   while (true) {
       select(...);
       ...
       if (timeout) {
           async_tick(&async);
       }
       async_process(&async);
   }

Native
------

NOTE: This runtime is only partly implemented and should not be used!

The native runtime implements all runtime features.

Typical usage:

.. code-block:: c

   async_init(&async);
   async_set_runtime(&async, async_runtime_create());
   ...
   async_run_forever(&async);

.. |buildstatus| image:: https://travis-ci.org/eerimoq/async.svg?branch=master
.. _buildstatus: https://travis-ci.org/eerimoq/async

.. |codecov| image:: https://codecov.io/gh/eerimoq/async/branch/master/graph/badge.svg
.. _codecov: https://codecov.io/gh/eerimoq/async

.. _libuv: https://github.com/libuv/libuv

.. _examples folder: https://github.com/eerimoq/async/tree/master/examples
