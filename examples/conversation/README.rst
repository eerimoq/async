About
=====

Bob asking you a few personal questions.

Compile and run
===============

.. code-block:: text

   $ make -s
   CC bob.c
   CC /home/erik/workspace/async/src/async_utils_linux.c
   CC /home/erik/workspace/async/src/async.c
   CC /home/erik/workspace/async/src/async_timer.c
   CC /home/erik/workspace/async/src/async_channel.c
   CC /home/erik/workspace/async/src/async_shell.c
   CC /home/erik/workspace/async/src/async_mqtt_client.c
   CC /home/erik/workspace/async/src/ports/none/async_core.c
   CC /home/erik/workspace/async/src/ports/none/async_tcp_client.c
   CC /home/erik/workspace/async/3pp/bitstream/bitstream.c
   CC /home/erik/workspace/async/3pp/humanfriendly/src/hf.c
   CC main.c
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
