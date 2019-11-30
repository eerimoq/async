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
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "async.h"
#include "internal.h"

void async_tcp_client_init(struct async_tcp_client_t *self_p,
                           async_func_t on_connect_complete,
                           async_func_t on_disconnected,
                           async_func_t on_data,
                           void *obj_p,
                           struct async_t *async_p)
{
    self_p->on_connect_complete = on_connect_complete;
    self_p->on_disconnected = on_disconnected;
    self_p->on_data = on_data;
    self_p->obj_p = obj_p;
    self_p->async_p = async_p;
    self_p->sockfd = -1;
}

void async_tcp_client_connect(struct async_tcp_client_t *self_p,
                              const char *host_p,
                              int port)
{
    async_tcp_client_connect_write(self_p, host_p, port);
}

void async_tcp_client_disconnect(struct async_tcp_client_t *self_p)
{
    async_tcp_client_disconnect_write(self_p);
}

bool async_tcp_client_is_connected(struct async_tcp_client_t *self_p)
{
    return (self_p->sockfd != -1);
}

ssize_t async_tcp_client_write(struct async_tcp_client_t *self_p,
                               const void *buf_p,
                               size_t size)
{
    return (write(self_p->sockfd, buf_p, size));
}

size_t async_tcp_client_read(struct async_tcp_client_t *self_p,
                             void *buf_p,
                             size_t size)
{
    ssize_t res;

    res = read(self_p->sockfd, buf_p, size);

    if (res == 0) {
        async_tcp_client_data_complete_write(self_p, true);
    } else if (res == -1) {
        async_tcp_client_data_complete_write(self_p, false);
        res = 0;
    } else {
        async_call(self_p->async_p,
                   self_p->on_data,
                   self_p->obj_p);
    }

    return (res);
}

void async_tcp_client_set_sockfd(struct async_tcp_client_t *self_p,
                                 int sockfd)
{
    self_p->sockfd = sockfd;
}
