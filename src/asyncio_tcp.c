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
#include <string.h>
#include <unistd.h>
#include "asyncio_tcp.h"
#include "internalio.h"

void asyncio_tcp_init(struct asyncio_tcp_t *self_p,
                      async_func_t on_connect_complete,
                      async_func_t on_data,
                      void *obj_p,
                      struct asyncio_t *asyncio_p)
{
    self_p->on_connect_complete = on_connect_complete;
    self_p->on_data = on_data;
    self_p->obj_p = obj_p;
    self_p->asyncio_p = asyncio_p;
}

void asyncio_tcp_connect(struct asyncio_tcp_t *self_p,
                         const char *host_p,
                         int port)
{
    asyncio_tcp_connect_write(self_p->asyncio_p, host_p, port);
}

void asyncio_tcp_disconnect(struct asyncio_tcp_t *self_p)
{
    asyncio_tcp_disconnect_write(self_p->asyncio_p, self_p->sock);
}

bool asyncio_tcp_is_connected(struct asyncio_tcp_t *self_p)
{
    return (self_p->sock != -1);
}

ssize_t asyncio_tcp_write(struct asyncio_tcp_t *self_p,
                          const void *buf_p,
                          size_t size)
{
    return (write(self_p->sock, buf_p, size));
}

size_t asyncio_tcp_read(struct asyncio_tcp_t *self_p,
                        void *buf_p,
                        size_t size)
{
    return (read(self_p->sock, buf_p, size));
}
