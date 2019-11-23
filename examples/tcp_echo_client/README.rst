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
   CC main.c
   CC /home/erik/workspace/async/src/asyncio_core.c
   CC /home/erik/workspace/async/src/asyncio_tcp.c
   CC /home/erik/workspace/async/src/asyncio_mqtt_client.c
   CC /home/erik/workspace/async/src/bitstream.c
   LD /home/erik/workspace/async/examples/tcp_echo_client/build/app
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
