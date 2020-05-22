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

/*
 * +--------+           +--------+
 * | client |           | server |
 * +--------+           +--------+
 *      |                    |
 *      |------- ping ------>|
 *      |<------ pong -------|
 *      |------- ping ------>|
 *      |<------ pong -------|
 *      .                    .
 *      .                    .
 *      .                    .
 */

#include "async.h"

struct client_t {
    struct server_t *server_p;
    struct async_t *async_p;
};

struct server_t {
    struct client_t *client_p;
    struct async_t *async_p;
};

static void client_on_pong(struct client_t *self_p, int *count_p);

static void server_init(struct server_t *self_p,
                        struct client_t *client_p,
                        struct async_t *async_p)
{
    self_p->client_p = client_p;
    self_p->async_p = async_p;
}

static void server_on_ping(struct server_t *self_p, int *count_p)
{
    printf("Ping count %d.\n", *count_p);

    *count_p += 2;

    async_call(self_p->async_p,
               (async_func_t)client_on_pong,
               self_p->client_p,
               count_p);
}

static void client_init(struct client_t *self_p,
                        struct server_t *server_p,
                        struct async_t *async_p)
{
    int *count_p;

    self_p->server_p = server_p;
    self_p->async_p = async_p;

    count_p = malloc(sizeof(*count_p));

    if (count_p == NULL) {
        exit(1);
    }

    *count_p = 1;

    async_call(async_p, (async_func_t)server_on_ping, server_p, count_p);
}

static void client_on_pong(struct client_t *self_p, int *count_p)
{
    printf("Pong count %d.\n", *count_p);

    *count_p -= 1;

    async_call(self_p->async_p,
               (async_func_t)server_on_ping,
               self_p->server_p,
               count_p);
}

int main()
{
    struct async_t async;
    struct client_t client;
    struct server_t server;

    async_init(&async);
    async_set_runtime(&async, async_runtime_create());
    server_init(&server, &client, &async);
    client_init(&client, &server, &async);
    async_run_forever(&async);

    return (0);
}
