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
#include <limits.h>
#include "asyncio.h"
#include "hf.h"

struct publisher_t {
    struct asyncio_mqtt_client_t client;
    struct async_timer_t publish_timer;
};

static void on_connected(struct publisher_t *self_p)
{
    printf("Connected.\n");
    async_timer_start(&self_p->publish_timer, 1000);
    asyncio_mqtt_client_subscribe(&self_p->client, "async/start");
    asyncio_mqtt_client_subscribe(&self_p->client, "async/stop");
}

static void on_disconnected(struct publisher_t *self_p)
{
    printf("Disconnected.\n");
    async_timer_stop(&self_p->publish_timer);
}

static void on_publish_start(struct publisher_t *self_p,
                             const uint8_t *buf_p,
                             size_t size)
{
    int timeout_ms;
    char buf[16];

    hf_buffer_to_string(&buf[0], sizeof(buf), buf_p, size);
    timeout_ms = hf_string_to_long(&buf[0], 100, INT_MAX, 1000, 10);
    printf("Starting the publish timer with timeout %d ms.\n", timeout_ms);
    async_timer_start(&self_p->publish_timer, timeout_ms);
}

static void on_publish_stop(struct publisher_t *self_p)
{
    printf("Stopping the publish timer.\n");
    async_timer_stop(&self_p->publish_timer);
}

static void on_publish(struct publisher_t *self_p,
                       const char *topic_p,
                       const uint8_t *buf_p,
                       size_t size)
{
    if (strcmp(topic_p, "async/start") == 0) {
        on_publish_start(self_p, buf_p, size);
    } else if (strcmp(topic_p, "async/stop") == 0) {
        on_publish_stop(self_p);
    }
}

static void on_publish_timeout(struct publisher_t *self_p)
{
    char buf[32];
    size_t size;
    static int counter = 0;

    size = sprintf(&buf[0], "count: %d", counter++);
    printf("Publishing '%s'.\n", &buf[0]);
    asyncio_mqtt_client_publish(&self_p->client, "async/hello", &buf[0], size);
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
                             (asyncio_mqtt_client_on_publish_t)on_publish,
                             &publisher,
                             &asyncio);
    asyncio_mqtt_client_set_client_id(&publisher.client, "mqtt-client-example");
    async_timer_init(&publisher.publish_timer,
                     (async_func_t)on_publish_timeout,
                     &publisher,
                     ASYNC_TIMER_PERIODIC,
                     &asyncio.async);
    printf("Starting the MQTT client.\n");
    asyncio_mqtt_client_start(&publisher.client);
    asyncio_run_forever(&asyncio);

    return (0);
}
