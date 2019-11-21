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

#ifndef ASYNCIO_TCP_H
#define ASYNCIO_TCP_H

#include "asyncio.h"

struct asyncio_tcp_t {
    async_func_t on_status_change;
    async_func_t on_data;
    void *obj_p;
    int sock;
};

/**
 * Initialize given TCP object.
 */
void asyncio_tcp_init(struct asyncio_tcp_t *self_p,
                      async_func_t on_status_change,
                      async_func_t on_data,
                      void *obj_p,
                      struct asyncio_t *asyncio_p);

/**
 * Opens a TCP connection to a remote host. The connection handle is
 * received in the response message.
 */
void asyncio_tcp_connect(struct asyncio_tcp_t *self_p,
                         const char *host_p,
                         int port);

/**
 * Disconnect from the remote host.
 */
void asyncio_tcp_disconnect(struct asyncio_tcp_t *self_p);

/**
 *
 */
bool asyncio_tcp_is_connected(struct asyncio_tcp_t *self_p);

/**
 * Write up to size bytes to the remote host.
 */
size_t asyncio_tcp_write(struct asyncio_tcp_t *self_p,
                         const void *buf_p,
                         size_t size);

/**
 * Read up to size bytes from the remote host.
 */
size_t asyncio_tcp_read(struct asyncio_tcp_t *self_p,
                        void *buf_p,
                        size_t size);

#endif
