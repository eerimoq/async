About
=====

Showcase the secure TCP client, both with and without TLS.

Compile and run
===============

First of all, start the TCP echo server in another terminal.

.. code-block:: text

   $ ./server.py

Then build and run the example program.

.. code-block:: text

   $ make -s
   ...
   tcp: Connecting to 'localhost:33000'...
   tcp: Connected.
   tcp: TX: 'Hello!'
   tcp: RX: 'He'
   tcp: RX: 'l'
   tcp: RX: ''
   tcp: RX: 'lo'
   tcp: RX: '!'
   tcp: RX: ''
   tcp: TX: 'Hello!'
   tcp: RX: 'He'
   tcp: RX: 'l'
   tcp: RX: 'l'
   tcp: RX: 'o!'
   tcp: RX: ''
   tcp: TX: 'Hello!'
   tcp: RX: 'He'
   tcp: RX: 'l'
   tcp: RX: 'lo!'
   tcp: RX: ''
