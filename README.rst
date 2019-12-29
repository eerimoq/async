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

- An MQTT client (only QoS 0 is supported).

- A simple shell.

- A basic TCP client.

Wish list
=========

- Secure TCP communication with TLS (OpenSSL, BearSSL and/or
  WolfSSL?).

- A basic UDP client.

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
       async_timer_init(&timer, on_timeout, 0, 1000, &async);
       async_timer_start(&timer);
       async_run_forever(&async);

       return (0);
   }

There are more examples in the `examples folder`_.

Runtimes
========

A runtime implements zero or more of the following features:

- Advance time.

- Networking (TCP).

- Call functions in the worker pool.

- Call functions from any thread.

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
       timeout_ms = async_process(&async);
       timer_update(timeout_ms);
       epoll_wait();
       ...
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

Unit testing
============

Source the development environment setup script.

.. code-block:: shell

   $ source setup.sh

Execute all unit tests.

.. code-block:: shell

   $ make -s -j4 test
   ...

Execute one test suite.

.. code-block:: shell

   $ make -s -j4 TEST=core_timer test
   ...

.. |buildstatus| image:: https://travis-ci.org/eerimoq/async.svg?branch=master
.. _buildstatus: https://travis-ci.org/eerimoq/async

.. |codecov| image:: https://codecov.io/gh/eerimoq/async/branch/master/graph/badge.svg
.. _codecov: https://codecov.io/gh/eerimoq/async

.. _libuv: https://github.com/libuv/libuv

.. _examples folder: https://github.com/eerimoq/async/tree/master/examples
