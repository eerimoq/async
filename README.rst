|buildstatus|_
|codecov|_
|nala|_

🔀 Async
=======

Asynchronous framework in C for systems where low memory usage is
important.

Features
========

- Delayed (asynchronous) function calls.

- Timers.

- An MQTT client (only QoS 0 is supported).

- A simple shell.

- A basic TCP client.

- Secure communication with SSL/TLS.

Project homepage: https://github.com/eerimoq/async

Releases: https://github.com/eerimoq/async/releases

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
       async_timer_init(&timer, on_timeout, NULL, 0, 1000, &async);
       async_timer_start(&timer);
       async_run_forever(&async);

       return (0);
   }

There are more examples in the `examples folder`_.

Runtimes
========

A runtime implements zero or more of the following features:

- Timer handling.

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
       epoll_wait(...);
       ...
       if (timeout) {
           async_tick(&async);
       }
       async_process(&async);
   }

Native
------

The native runtime implements all runtime features.

Typical usage:

.. code-block:: c

   async_init(&async);
   async_set_runtime(&async, async_runtime_create());
   ...
   async_run_forever(&async);

Design
======

Input
-----

First ``*_input(self_p)`` is called to signal that data is
available. Then read data with ``*_read(self_p, buf_p, size)``. The
read function must be called at least once per input call.

Output
------

Write data with ``*_write(self_p, buf_p, size)``.

Unit testing
============

Source the development environment setup script.

.. code-block:: shell

   $ source setup.sh

Execute all unit tests.

.. code-block:: shell

   $ make -s -j4 test
   ...

Execute tests matching given pattern.

.. code-block:: shell

   $ make -s -j4 ARGS=core_timer test
   ...

.. |buildstatus| image:: https://travis-ci.org/eerimoq/async.svg?branch=master
.. _buildstatus: https://travis-ci.org/eerimoq/async

.. |codecov| image:: https://codecov.io/gh/eerimoq/async/branch/master/graph/badge.svg
.. _codecov: https://codecov.io/gh/eerimoq/async

.. |nala| image:: https://img.shields.io/badge/nala-test-blue.svg
.. _nala: https://github.com/eerimoq/nala

.. _examples folder: https://github.com/eerimoq/async/tree/master/examples
