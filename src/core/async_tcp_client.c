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
#include <stdlib.h>
#include "async/core.h"
#include "async/core/runtime.h"

static void on_connected_default(struct async_tcp_client_t *self_p, int res)
{
    (void)self_p;
    (void)res;
}

static void on_disconnected_default(struct async_tcp_client_t *self_p)
{
    (void)self_p;
}

static void on_input_default(struct async_tcp_client_t *self_p)
{
    char buf[32];
    size_t size;

    do {
        size = async_tcp_client_read(self_p, &buf[0], sizeof(buf));
    } while (size > 0);
}

void async_tcp_client_init(struct async_tcp_client_t *self_p,
                           async_tcp_client_connected_t on_connected,
                           async_tcp_client_disconnected_t on_disconnected,
                           async_tcp_client_input_t on_input,
                           struct async_t *async_p)
{
    if (on_connected == NULL) {
        on_connected = on_connected_default;
    }

    if (on_disconnected == NULL) {
        on_disconnected = on_disconnected_default;
    }

    if (on_input == NULL) {
        on_input = on_input_default;
    }

    self_p->async_p = async_p;
    async_p->runtime_p->tcp_client.init(self_p,
                                        on_connected,
                                        on_disconnected,
                                        on_input);
}

void async_tcp_client_connect(struct async_tcp_client_t *self_p,
                              const char *host_p,
                              int port)
{
    self_p->async_p->runtime_p->tcp_client.connect(self_p, host_p, port);
}

void async_tcp_client_disconnect(struct async_tcp_client_t *self_p)
{
    self_p->async_p->runtime_p->tcp_client.disconnect(self_p);
}

void async_tcp_client_write(struct async_tcp_client_t *self_p,
                            const void *buf_p,
                            size_t size)
{
    self_p->async_p->runtime_p->tcp_client.write(self_p, buf_p, size);
}

size_t async_tcp_client_read(struct async_tcp_client_t *self_p,
                             void *buf_p,
                             size_t size)
{
    return (self_p->async_p->runtime_p->tcp_client.read(self_p, buf_p, size));
}
