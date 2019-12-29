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
    printf("Connecting to 'localhost:%d'...\n", self_p->port);
    async_tcp_client_connect(&self_p->tcp, "localhost", self_p->port);
}

static void on_start(struct echo_client_t *self_p)
{
    do_connect(self_p);
}

static void on_connected(struct async_tcp_client_t *tcp_p, int res)
{
    struct echo_client_t *self_p;

    self_p = async_container_of(tcp_p, typeof(*self_p), tcp);

    if (res == 0) {
        printf("Connected.\n");
        async_timer_start(&self_p->transmit_timer);
    } else {
        printf("Connect failed.\n");
        async_timer_start(&self_p->reconnect_timer);
    }
}

static void on_disconnected(struct async_tcp_client_t *tcp_p)
{
    struct echo_client_t *self_p;

    self_p = async_container_of(tcp_p, typeof(*self_p), tcp);
    printf("Disconnected.\n");
    async_timer_stop(&self_p->transmit_timer);
    async_timer_start(&self_p->reconnect_timer);
}

static void on_input(struct async_tcp_client_t *tcp_p)
{
    char buf[8];
    ssize_t size;
    struct echo_client_t *self_p;

    self_p = async_container_of(tcp_p, typeof(*self_p), tcp);
    size = async_tcp_client_read(&self_p->tcp, &buf[0], sizeof(buf));

    printf("RX: '");
    fwrite(&buf[0], 1, size, stdout);
    printf("'\n");
}

static void on_transmit_timeout(struct async_timer_t *timer_p)
{
    struct echo_client_t *self_p;

    self_p = async_container_of(timer_p, typeof(*self_p), transmit_timer);
    printf("TX: 'Hello!'\n");
    async_tcp_client_write(&self_p->tcp, "Hello!", 6);
}

static void on_reconnect_timeout(struct async_timer_t *timer_p)
{
    struct echo_client_t *self_p;

    self_p = async_container_of(timer_p, typeof(*self_p), reconnect_timer);
    do_connect(self_p);
}

void echo_client_init(struct echo_client_t *self_p,
                      int port,
                      struct async_t *async_p)
{
    self_p->port = port;
    async_tcp_client_init(&self_p->tcp,
                          on_connected,
                          on_disconnected,
                          on_input,
                          async_p);
    async_timer_init(&self_p->transmit_timer,
                     on_transmit_timeout,
                     0,
                     1000,
                     async_p);
    async_timer_init(&self_p->reconnect_timer,
                     on_reconnect_timeout,
                     1000,
                     0,
                     async_p);
    async_call(async_p, (async_func_t)on_start, self_p);
}
