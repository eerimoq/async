About
=====

A basic HTTP client.

Compile and run
===============

First of all, start the HTTP server in another terminal.

.. code-block:: text

   $ python3 -m http.server 8080

Then build and run the example program.

.. code-block:: text

   $ make -s
   ...
   Getting 'http://localhost:8080/main.c'...
   --------------------- HTTP GET BEGIN ---------------------
   HTTP/1.0 200 OK
   Server: SimpleHTTP/0.6 Python/3.7.3
   Date: Tue, 24 Dec 2019 18:45:26 GMT
   Content-type: text/plain
   Content-Length: 1480
   Last-Modified: Tue, 24 Dec 2019 18:16:37 GMT

   /*
    * The MIT License (MIT)
    *
    * Copyright (c) 2019, Erik Moqvist
    *
    * Permission is hereby granted, free of charge, to any person
    * obtaining a copy of this software and associated documentation
    * files (the "Software"), to deal in the Software without
    * restriction, including without limitation the rights to use, copy,
    * modify, merge, publish, distribute, sublicense, and/or sell copies
    * of the Software, and to permit persons to whom the Software is
    * furnished to do so, subject to the following conditions:
    *
    * The above copyright notice and this permission notice shall be
    * included in all copies or substantial portions of the Software.
    *
    * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
    * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
    * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    * SOFTWARE.
    *
    * This file is part of the Async project.
    */

   #include "async.h"
   #include "http_get.h"

   int main()
   {
       struct async_t async;
       struct http_get_t http_get;

       async_init(&async);
       async_set_runtime(&async, async_runtime_create());
       http_get_init(&http_get, &async);
       async_run_forever(&async);

       return (0);
   }
   ---------------------- HTTP GET END ----------------------

Create key and certificate
==========================

.. code-block:: text

   $ openssl genrsa -des3 -out server.orig.key 2048
   $ openssl rsa -in server.orig.key -out server.key
   $ openssl req -new -key server.key -out server.csr
   $ openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt
