About
=====

Showcase the TCP client.

Compile and run
===============

First of all, start the TCP echo server in another terminal.

.. code-block:: text

   $ ./server.py

Then build and run the example program.

.. code-block:: text

   $ make -s
   ...
   Connecting to 'localhost:33000'...
   Connected.
   TX: 'Hello!'
   RX: 'He'
   RX: 'l'
   RX: ''
   RX: 'lo'
   RX: '!'
   RX: ''
   TX: 'Hello!'
   RX: 'He'
   RX: 'l'
   RX: 'l'
   RX: 'o!'
   RX: ''
   TX: 'Hello!'
   RX: 'He'
   RX: 'l'
   RX: 'lo!'
   RX: ''
