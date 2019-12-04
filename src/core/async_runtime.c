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
#include "async/runtimes/linux.h"

static void set_async(void *self_p, struct async_t *async_p)
{
    (void)self_p;
    (void)async_p;

    fprintf(stderr, "set_async() not implemented.\n");
    exit(1);
}

static void run_forever(void *self_p)
{
    (void)self_p;

    fprintf(stderr, "async_run_forever() not implemented.\n");
    exit(1);
}

static void tcp_client_init(struct async_tcp_client_t *self_p,
                            async_tcp_client_connected_t on_connected,
                            async_func_t on_disconnected,
                            async_func_t on_data,
                            void *obj_p)
{
    (void)self_p;
    (void)on_connected;
    (void)on_disconnected;
    (void)on_data;
    (void)obj_p;

    fprintf(stderr, "async_tcp_client_init() not implemented.\n");
    exit(1);
}

static void tcp_client_connect(struct async_tcp_client_t *self_p,
                               const char *host_p,
                               int port)
{
    (void)self_p;
    (void)host_p;
    (void)port;

    fprintf(stderr, "async_tcp_client_connect() not implemented.\n");
    exit(1);
}

static void tcp_client_disconnect(struct async_tcp_client_t *self_p)
{
    (void)self_p;

    fprintf(stderr, "async_tcp_client_disconnect() not implemented.\n");
    exit(1);
}

static ssize_t tcp_client_write(struct async_tcp_client_t *self_p,
                                const void *buf_p,
                                size_t size)
{
    (void)self_p;
    (void)buf_p;
    (void)size;

    fprintf(stderr, "async_tcp_client_write() not implemented.\n");
    exit(1);

    return (-1);
}

static size_t tcp_client_read(struct async_tcp_client_t *self_p,
                              void *buf_p,
                              size_t size)
{
    (void)self_p;
    (void)buf_p;
    (void)size;

    fprintf(stderr, "async_tcp_client_read() not implemented.\n");
    exit(1);

    return (0);
}

struct async_runtime_t runtime = {
    .set_async = set_async,
    .run_forever = run_forever,
    .tcp_client = {
        .init = tcp_client_init,
        .connect = tcp_client_connect,
        .disconnect = tcp_client_disconnect,
        .write = tcp_client_write,
        .read = tcp_client_read
    }
};

struct async_runtime_t *async_runtime_create(void)
{
    return (async_runtime_linux_create());
}

struct async_runtime_t *async_runtime_null_create()
{
    return (&runtime);
}
