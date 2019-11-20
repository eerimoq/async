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
 * Publishes a message periodically.
 */

#include <stdio.h>
#include <string.h>
#include "async.h"
#include "async_linux.h"

static struct async_mqtt_client_t client;

static ASYNC_UID_DEFINE(timeout);

static void handle_timeout(struct async_task_t *self_p)
{
    struct async_mqtt_client_message_publish_t *publish_p;
    const char *topic_p = "async/examples/mqtt_client";
    const char *message_p = "Hello world!";

    if (!async_mqtt_client_is_connected(&client)) {
        printf("MQTT client is not connected. Not publishing...\n");
        return;
    }

    printf("Publishing '%s' on '%s'...\n", message_p, topic_p);

    publish_p = async_message_alloc(
        self_p->async_p,
        &async_mqtt_client_message_id_publish,
        sizeof(struct async_mqtt_client_message_publish_t));
    publish_p->topic_p = topic_p;
    publish_p->message_p = (uint8_t *)message_p;
    publish_p->size = strlen(message_p);
    async_send(async_mqtt_client_get_task(&client), publish_p);
}

static void publisher_on_message(struct async_task_t *self_p,
                                 struct async_uid_t *uid_p,
                                 void *message_p)
{
    (void)message_p;

    if (uid_p == &timeout) {
        handle_timeout(self_p);
    }
}

int main()
{
    struct async_t async;
    struct async_task_t publisher;
    void *start_p;
    struct async_timer_t timer;
    struct async_linux_t async_linux;

    /* Setup. */
    async_init(&async, 100, NULL, 0);
    async_mqtt_client_init(&client, &async, "localhost", 1883);
    async_task_init(&publisher, &async, publisher_on_message);
    async_timer_init(&timer,
                     &async,
                     1000,
                     &timeout,
                     &publisher,
                     ASYNC_TIMER_PERIODIC);
    async_timer_start(&timer);
    start_p = async_message_alloc(&async,
                                  &async_mqtt_client_message_id_start,
                                  0);
    async_send(async_mqtt_client_get_task(&client), start_p);

    /* Start. */
    async_linux_create(&async_linux, &async);
    async_linux_join(&async_linux);

    return (0);
}
