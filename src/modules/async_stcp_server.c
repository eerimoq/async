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

#include "async/modules/stcp_server.h"

void async_stcp_server_init(struct async_stcp_server_t *self_p,
                            struct async_ssl_context_t *ssl_context_p,
                            async_stcp_server_client_connected_t on_connected,
                            async_stcp_server_client_disconnected_t on_disconnected,
                            async_stcp_server_client_input_t on_input,
                            struct async_t *async_p)
{
    (void)self_p;
    (void)ssl_context_p;
    (void)on_connected;
    (void)on_disconnected;
    (void)on_input;
    (void)async_p;
}

void async_stcp_server_add_client(struct async_stcp_server_t *self_p,
                                  struct async_stcp_server_client_t *client_p)
{
    (void)self_p;
    (void)client_p;
}

void async_stcp_server_start(struct async_stcp_server_t *self_p)
{
    (void)self_p;
}

void async_stcp_server_stop(struct async_stcp_server_t *self_p)
{
    (void)self_p;
}

void async_stcp_server_client_write(struct async_stcp_server_client_t *self_p,
                                    const void *buf_p,
                                    size_t size)
{
    (void)self_p;
    (void)buf_p;
    (void)size;
}

size_t async_stcp_server_client_read(struct async_stcp_server_client_t *self_p,
                                     void *buf_p,
                                     size_t size)
{
    (void)self_p;
    (void)buf_p;
    (void)size;

    return (0);
}

void async_stcp_server_client_disconnect(struct async_stcp_server_client_t *self_p)
{
    (void)self_p;
}
