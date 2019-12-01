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

#ifndef ASYNC_TCP_CLIENT_H
#define ASYNC_TCP_CLIENT_H

#include "async/core.h"

/**
 * Initialize given TCP client object.
 */
void async_tcp_client_init(struct async_tcp_client_t *self_p,
                           async_func_t on_connect_complete,
                           async_func_t on_disconnected,
                           async_func_t on_input,
                           void *obj_p,
                           struct async_t *async_p);

/**
 * Opens a TCP connection to a remote host. on_connect_complete is
 * called once completed.
 */
void async_tcp_client_connect(struct async_tcp_client_t *self_p,
                              const char *host_p,
                              int port);

/**
 * Disconnect from the remote host.
 */
void async_tcp_client_disconnect(struct async_tcp_client_t *self_p);

/**
 * Returns true if connected to the peer.
 */
bool async_tcp_client_is_connected(struct async_tcp_client_t *self_p);

/**
 * Write up to size bytes to the remote host.
 */
ssize_t async_tcp_client_write(struct async_tcp_client_t *self_p,
                               const void *buf_p,
                               size_t size);

/**
 * Read up to size bytes from the remote host.
 */
size_t async_tcp_client_read(struct async_tcp_client_t *self_p,
                             void *buf_p,
                             size_t size);

#endif
