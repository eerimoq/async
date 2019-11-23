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

struct publisher_t {
    struct asyncio_mqtt_client_t client;
    struct async_timer_t publish_timer;
};

static void on_connected(struct publisher_t *self_p)
{
    printf("Connected.\n");
    async_timer_start(&self_p->publish_timer);
}

static void on_disconnected(struct publisher_t *self_p)
{
    printf("Disconnected.\n");
    async_timer_stop(&self_p->publish_timer);
}

static void on_publish(struct publisher_t *self_p)
{
    struct asyncio_mqtt_client_message_t *message_p;

    message_p = asyncio_mqtt_client_get_message(&self_p->client);
    printf("Got message on topic '%s'.\n", message_p->topic_p);
    asyncio_mqtt_client_message_free(message_p);
}

static void on_timeout(struct publisher_t *self_p)
{
    printf("Publishing.\n");
    asyncio_mqtt_client_publish(&self_p->client,
                                "async/examples/mqtt_client",
                                "Hello world!",
                                12);
}

int main()
{
    struct asyncio_t asyncio;
    struct publisher_t publisher;

    asyncio_init(&asyncio);
    asyncio_mqtt_client_init(&publisher.client,
                             "localhost",
                             1883,
                             (async_func_t)on_connected,
                             (async_func_t)on_disconnected,
                             (async_func_t)on_publish,
                             &publisher,
                             &asyncio);
    async_timer_init(&publisher.publish_timer,
                     1000,
                     (async_func_t)on_timeout,
                     &publisher,
                     ASYNC_TIMER_PERIODIC,
                     &asyncio.async);
    asyncio_mqtt_client_start(&publisher.client);
    asyncio_run_forever(&asyncio);

    return (0);
}
