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
#include "asyncio.h"

struct echo_client_t {
    struct async_timer_t timer;
    struct asyncio_tcp_t tcp;
};

static void on_start(struct echo_client_t *self_p)
{
    asyncio_tcp_connect(&self_p->tcp, "localhost", 33000);
}

static void on_tcp_connect_complete(struct echo_client_t *self_p)
{
    if (asyncio_tcp_is_connected(&self_p->tcp)) {
        async_timer_start(&self_p->timer);
    } else {
        printf("Connect failed.\n");
    }
}

static void on_tcp_data(struct echo_client_t *self_p)
{
    char buf[32];
    size_t size;

    size = asyncio_tcp_read(&self_p->tcp, &buf[0], sizeof(buf));

    if (size > 0) {
        printf("RX: '");
        fwrite(&buf[0], size, 1, stdout);
        printf("'\n");
    } else {
        printf("Connection closed.\n");
        async_timer_stop(&self_p->timer);
    }
}

static void on_timeout(struct echo_client_t *self_p)
{
    if (asyncio_tcp_is_connected(&self_p->tcp)) {
        printf("TX: 'Hello!'\n");
        asyncio_tcp_write(&self_p->tcp, "Hello!", 6);
    }
}

int main()
{
    struct asyncio_t asyncio;
    struct echo_client_t echo_client;

    asyncio_init(&asyncio);
    asyncio_tcp_init(&tcp,
                     (async_func_t)on_tcp_connect_complete,
                     (async_func_t)on_tcp_data,
                     &echo_client,
                     &asyncio);
    async_timer_init(&timer,
                     1000,
                     (async_func_t)on_timeout,
                     &echo_client,
                     ASYNC_TIMER_PERIODIC,
                     &asyncio.async);
    async_call(&asyncio.async, (async_func_t)on_start, &echo_client);
    asyncio_run_forever(&asyncio);

    return (0);
}
