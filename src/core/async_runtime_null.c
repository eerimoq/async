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

#include <stdio.h>
#include "async/core/runtime.h"

static void set_async()
{
    fprintf(stderr, "async_set_runtime() not implemented.\n");
    exit(1);
}

static void call_threadsafe()
{
    fprintf(stderr, "async_call_threadsafe() not implemented.\n");
    exit(1);
}

static int call_worker_pool()
{
    fprintf(stderr, "async_call_worker_pool() not implemented.\n");
    exit(1);
}

static void run_forever()
{
    fprintf(stderr, "async_run_forever() not implemented.\n");
    exit(1);
}

static void tcp_client_init()
{
    fprintf(stderr, "async_tcp_client_init() not implemented.\n");
    exit(1);
}

static void tcp_client_connect()
{
    fprintf(stderr, "async_tcp_client_connect() not implemented.\n");
    exit(1);
}

static void tcp_client_disconnect()
{
    fprintf(stderr, "async_tcp_client_disconnect() not implemented.\n");
    exit(1);
}

static void tcp_client_write()
{
    fprintf(stderr, "async_tcp_client_write() not implemented.\n");
    exit(1);
}

static size_t tcp_client_read()
{
    fprintf(stderr, "async_tcp_client_read() not implemented.\n");
    exit(1);

    return (0);
}

static void tcp_server_init()
{
    fprintf(stderr, "async_tcp_server_init() not implemented.\n");
    exit(1);
}

static void tcp_server_add_client()
{
    fprintf(stderr, "async_tcp_server_add_client() not implemented.\n");
    exit(1);
}

static int tcp_server_start()
{
    fprintf(stderr, "async_tcp_server_start() not implemented.\n");
    exit(1);

    return (0);
}

static void tcp_server_stop()
{
    fprintf(stderr, "async_tcp_server_stop() not implemented.\n");
    exit(1);
}

static void tcp_server_client_write()
{
    fprintf(stderr, "async_tcp_server_client_write() not implemented.\n");
    exit(1);
}

static size_t tcp_server_client_read()
{
    fprintf(stderr, "async_tcp_server_client_read() not implemented.\n");
    exit(1);

    return (0);
}

static void tcp_server_client_disconnect()
{
    fprintf(stderr, "async_tcp_server_client_disconnect() not implemented.\n");
    exit(1);
}

static struct async_runtime_t runtime = {
    .set_async = set_async,
    .call_threadsafe = call_threadsafe,
    .call_worker_pool = call_worker_pool,
    .run_forever = run_forever,
    .tcp_client = {
        .init = tcp_client_init,
        .connect = tcp_client_connect,
        .disconnect = tcp_client_disconnect,
        .write = tcp_client_write,
        .read = tcp_client_read
    },
    .tcp_server = {
        .init = tcp_server_init,
        .add_client = tcp_server_add_client,
        .start = tcp_server_start,
        .stop = tcp_server_stop,
        .client = {
            .write = tcp_server_client_write,
            .read = tcp_server_client_read,
            .disconnect = tcp_server_client_disconnect
        }
    }
};

struct async_runtime_t *async_runtime_null_create()
{
    return (&runtime);
}
