About
=====

Print 'Hello world!' periodically.

Compile and run
===============

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
   LD /home/erik/workspace/async/examples/hello_world/build/app
   Hello world!
   Hello world!
   Hello world!
   Hello world!
