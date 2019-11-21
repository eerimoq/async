😺 Async
=======

Asynchronous framework in C for systems where low memory usage is
important.

Features
========

- Delayed function calls.

- Timers.

Examples
========

There are a few example in the `examples folder`_.

Linux design
============

Two threads:

- Async thread - executing async functions

- IO thread - performing IO and timing

.. _Monolinux C library: https://github.com/eerimoq/monolinux-c-library

.. _examples folder: https://github.com/eerimoq/async/tree/master/examples
