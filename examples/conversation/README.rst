About
=====

Bob asking you a few personal questions.

Compile and run
===============

.. code-block:: text

   $ make -s
   CC main.c
   CC /home/erik/workspace/async/src/async.c
   CC /home/erik/workspace/async/src/async_timer.c
   CC /home/erik/workspace/async/src/async_linux.c
   CC /home/erik/workspace/async/src/asyncio_core.c
   CC /home/erik/workspace/async/src/asyncio_tcp.c
   CC /home/erik/workspace/async/src/asyncio_mqtt_client.c
   CC /home/erik/workspace/async/src/bitstream.c
   LD /home/erik/workspace/async/examples/conversation/build/app
   Bob: Hello!
   You: Hi!
   Bob: What is your name?
   You: Erik
   Bob: How old are you?
   Erik:
   Bob: Do you hear me?
   Erik: Yes
   Bob: How old are you?
   Erik: 22
   Bob: That's it, thanks!

   Bob: Hello!
   You:
