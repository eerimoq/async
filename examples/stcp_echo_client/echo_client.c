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
#include "echo_client.h"

static void do_connect(struct echo_client_t *self_p)
{
    printf("%s: Connecting to 'localhost:%d'...\n", self_p->name_p, self_p->port);
    async_stcp_client_connect(&self_p->stcp, "localhost", self_p->port);
}

static void on_start(struct echo_client_t *self_p)
{
    do_connect(self_p);
}

static void on_connected(struct async_stcp_client_t *stcp_p, int res)
{
    struct echo_client_t *self_p;

    self_p = async_container_of(stcp_p, typeof(*self_p), stcp);

    if (res == 0) {
        printf("%s: Connected.\n", self_p->name_p);
        async_timer_start(&self_p->transmit_timer);
    } else {
        printf("%s: Connect failed.\n", self_p->name_p);
        async_timer_start(&self_p->reconnect_timer);
    }
}

static void on_disconnected(struct async_stcp_client_t *stcp_p)
{
    struct echo_client_t *self_p;

    self_p = async_container_of(stcp_p, typeof(*self_p), stcp);
    printf("%s: Disconnected.\n", self_p->name_p);
    async_timer_stop(&self_p->transmit_timer);
    async_timer_start(&self_p->reconnect_timer);
}

static void on_input(struct async_stcp_client_t *stcp_p)
{
    char buf[8];
    ssize_t size;
    struct echo_client_t *self_p;

    self_p = async_container_of(stcp_p, typeof(*self_p), stcp);
    size = async_stcp_client_read(&self_p->stcp, &buf[0], sizeof(buf));

    if (size > 0) {
        printf("%s: RX: '", self_p->name_p);
        fwrite(&buf[0], 1, size, stdout);
        printf("'\n");
    }
}

static void on_transmit_timeout(struct echo_client_t *self_p)
{
    printf("%s: TX: 'Hello!'\n", self_p->name_p);
    async_stcp_client_write(&self_p->stcp, "Hello!", 6);
}

static void on_reconnect_timeout(struct echo_client_t *self_p)
{
    do_connect(self_p);
}

void echo_client_init(struct echo_client_t *self_p,
                      const char *name_p,
                      int port,
                      struct async_ssl_context_t *ssl_context_p,
                      struct async_t *async_p)
{
    self_p->name_p = name_p;
    self_p->port = port;
    async_stcp_client_init(&self_p->stcp,
                           ssl_context_p,
                           on_connected,
                           on_disconnected,
                           on_input,
                           async_p);
    async_timer_init(&self_p->transmit_timer,
                     (async_timer_timeout_t)on_transmit_timeout,
                     self_p,
                     0,
                     1000,
                     async_p);
    async_timer_init(&self_p->reconnect_timer,
                     (async_timer_timeout_t)on_reconnect_timeout,
                     self_p,
                     1000,
                     0,
                     async_p);
    async_call(async_p, (async_func_t)on_start, self_p);
}
