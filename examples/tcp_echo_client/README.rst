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
   CC /home/erik/workspace/async/src/async_utils_linux.c
   CC /home/erik/workspace/async/src/async.c
   CC /home/erik/workspace/async/src/async_timer.c
   CC /home/erik/workspace/async/src/async_channel.c
   CC /home/erik/workspace/async/src/async_shell.c
   CC /home/erik/workspace/async/src/async_mqtt_client.c
   CC /home/erik/workspace/async/src/ports/linux/async_core.c
   CC /home/erik/workspace/async/src/ports/linux/async_tcp_client.c
   CC /home/erik/workspace/async/3pp/bitstream/bitstream.c
   CC /home/erik/workspace/async/3pp/humanfriendly/src/hf.c
   CC main.c
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
