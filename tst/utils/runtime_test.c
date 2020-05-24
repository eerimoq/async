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
#include "nala.h"
#include "async/core/runtime.h"
#include "async/runtimes/linux.h"
#include "runtime_test.h"

static void set_async(void *self_p, struct async_t *async_p)
{
    runtime_test_set_async(self_p, async_p);
}

static void run_forever(void *self_p)
{
    runtime_test_run_forever(self_p);
}

static void tcp_client_init(struct async_tcp_client_t *self_p,
                            async_tcp_client_connected_t on_connected,
                            async_tcp_client_disconnected_t on_disconnected,
                            async_tcp_client_input_t on_input)
{
    runtime_test_tcp_client_init(self_p,
                                 on_connected,
                                 on_disconnected,
                                 on_input);
}

static void tcp_client_connect(struct async_tcp_client_t *self_p,
                               const char *host_p,
                               int port)
{
    runtime_test_tcp_client_connect(self_p, host_p, port);
}

static void tcp_client_disconnect(struct async_tcp_client_t *self_p)
{
    runtime_test_tcp_client_disconnect(self_p);
}

static void tcp_client_write(struct async_tcp_client_t *self_p,
                             const void *buf_p,
                             size_t size)
{
    runtime_test_tcp_client_write(self_p, buf_p, size);
}

static size_t tcp_client_read(struct async_tcp_client_t *self_p,
                              void *buf_p,
                              size_t size)
{
    return (runtime_test_tcp_client_read(self_p, buf_p, size));
}

static void tcp_server_init(struct async_tcp_server_t *self_p)
{
    (void)self_p;
}

static void tcp_server_start(struct async_tcp_server_t *self_p)
{
    (void)self_p;
}

static void tcp_server_stop(struct async_tcp_server_t *self_p)
{
    (void)self_p;
}

static void tcp_server_client_write(struct async_tcp_server_client_t *self_p,
                                    const void *buf_p,
                                    size_t size)
{
    (void)self_p;
    (void)buf_p;
    (void)size;
}

static size_t tcp_server_client_read(struct async_tcp_server_client_t *self_p,
                                     void *buf_p,
                                     size_t size)
{
    (void)self_p;
    (void)buf_p;
    (void)size;

    return (0);
}

static void tcp_server_client_disconnect(struct async_tcp_server_client_t *self_p)
{
    (void)self_p;
}

static struct async_runtime_t runtime = {
    .set_async = set_async,
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
        .start = tcp_server_start,
        .stop = tcp_server_stop,
        .client = {
            .write = tcp_server_client_write,
            .read = tcp_server_client_read,
            .disconnect = tcp_server_client_disconnect
        }
    }
};

struct async_runtime_t *runtime_test_create()
{
    return (&runtime);
}
