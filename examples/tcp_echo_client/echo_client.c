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

struct echo_client_t {
    struct async_tcp_client_t tcp;
    struct async_timer_t transmit_timer;
    struct async_timer_t reconnect_timer;
};

static void on_start(struct echo_client_t *self_p)
{
    printf("Connecting to 'localhost:33000'...\n");
    async_tcp_client_connect(&self_p->tcp, "localhost", 33000);
}

static void on_connect_complete(struct echo_client_t *self_p)
{
    if (async_tcp_client_is_connected(&self_p->tcp)) {
        printf("Connected.\n");
        async_timer_start(&self_p->transmit_timer, 1000);
    } else {
        printf("Connect failed.\n");
        async_timer_start(&self_p->reconnect_timer, 1000);
    }
}

static void on_disconnected(struct echo_client_t *self_p)
{
    printf("Disconnected.\n");
    async_timer_stop(&self_p->transmit_timer);
    async_timer_start(&self_p->reconnect_timer, 1000);
}

static void on_data(struct echo_client_t *self_p)
{
    char buf[8];
    ssize_t size;

    size = async_tcp_client_read(&self_p->tcp, &buf[0], sizeof(buf));

    printf("RX: '");
    fwrite(&buf[0], 1, size, stdout);
    printf("'\n");
}

static void on_timeout(struct echo_client_t *self_p)
{
    printf("TX: 'Hello!'\n");
    async_tcp_client_write(&self_p->tcp, "Hello!", 6);
}

void echo_client_init(struct echo_client_t *self_p, struct async_t *async_p)
{
    async_tcp_client_init(&self_p->tcp,
                          (async_func_t)on_connect_complete,
                          (async_func_t)on_disconnected,
                          (async_func_t)on_data,
                          self_p,
                          async_p);
    async_timer_init(&self_p->transmit_timer,
                     (async_func_t)on_timeout,
                     self_p,
                     ASYNC_TIMER_PERIODIC,
                     async_p);
    async_timer_init(&self_p->reconnect_timer,
                     (async_func_t)on_start,
                     self_p,
                     0,
                     async_p);
    async_call(async_p, (async_func_t)on_start, self_p);
}
