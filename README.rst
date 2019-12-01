|buildstatus|_
|codecov|_

🔀 Async
=======

Asynchronous framework in C for systems where low memory usage is
important.

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
       async_timer_init(&timer, on_timeout, NULL, ASYNC_TIMER_PERIODIC, &async);
       async_timer_start(&timer, 1000);
       async_run_forever(&async);

       return (0);
   }

There are more examples in the `examples folder`_.

Ports
=====

Features that requires a port:

- TCP.

Available ports:

- None. Port features listed above can't be used.

- Linux.

.. |buildstatus| image:: https://travis-ci.org/eerimoq/async.svg?branch=master
.. _buildstatus: https://travis-ci.org/eerimoq/async

.. |codecov| image:: https://codecov.io/gh/eerimoq/async/branch/master/graph/badge.svg
.. _codecov: https://codecov.io/gh/eerimoq/async

.. _examples folder: https://github.com/eerimoq/async/tree/master/examples
