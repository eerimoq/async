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
#include "async.h"
#include "echo_server.h"

static void on_client_connected()
{
    printf("Client connected!\n");
}

static void on_client_disconnected()
{
    printf("Client disconnected!\n");
}

static void on_client_input(struct async_stcp_server_client_t *client_p)
{
    char buf[16];
    size_t size;

    while (true) {
        size = async_stcp_server_client_read(client_p, &buf[0], sizeof(buf));

        if (size == 0) {
            break;
        }

        async_stcp_server_client_write(client_p, &buf[0], size);
    }
}

void echo_server_init(struct echo_server_t *self_p,
                      const char *name_p,
                      int port,
                      struct async_ssl_context_t *ssl_context_p,
                      struct async_t *async_p)
{
    self_p->name_p = name_p;
    self_p->port = port;
    async_stcp_server_init(&self_p->stcp,
                           ssl_context_p,
                           on_client_connected,
                           on_client_disconnected,
                           on_client_input,
                           async_p);
    async_stcp_server_add_client(&self_p->stcp, &self_p->client);
    async_stcp_server_start(&self_p->stcp);
}
