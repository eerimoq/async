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

#ifndef ASYNC_MQTT_CLIENT_H
#define ASYNC_MQTT_CLIENT_H

#include "async_core.h"

struct async_mqtt_client_t {
    struct async_task_t task;
    struct async_t *async_p;
    const char *host_p;
    int port;
    const char *client_id_p;
    int keep_alive_s;
    int response_timeout;
    int session_expiry_interval;
    bool connected;
};

struct async_mqtt_client_message_subscribe_t {
    const char *topic_p;
};

struct async_mqtt_client_message_unsubscribe_t {
    const char *topic_p;
};

struct async_mqtt_client_message_publish_t {
    const char *topic_p;
    const uint8_t *message_p;
    size_t size;
};

struct async_mqtt_client_message_on_publish_t {
    const char *topic_p;
    const uint8_t *message_p;
    size_t size;
};

ASYNC_UID(async_mqtt_client_message_id_start);
ASYNC_UID(async_mqtt_client_message_id_stop);
ASYNC_UID(async_mqtt_client_message_id_subscribe);
ASYNC_UID(async_mqtt_client_message_id_unsubscribe);
ASYNC_UID(async_mqtt_client_message_id_publish);
ASYNC_UID(async_mqtt_client_message_id_connected);
ASYNC_UID(async_mqtt_client_message_id_disconnected);

void async_mqtt_client_init(struct async_mqtt_client_t *self_p,
                            struct async_t *async_p,
                            const char *host_p,
                            int port);

void async_mqtt_client_set_client_id(struct async_mqtt_client_t *self_p,
                                     const char *client_id_p);

void async_mqtt_client_set_response_timeout(struct async_mqtt_client_t *self_p,
                                            int response_timeout);

void async_mqtt_client_set_session_expiry_interval(struct async_mqtt_client_t *self_p,
                                                   int session_expiry_interval);

struct async_task_t *async_mqtt_client_get_task(struct async_mqtt_client_t *self_p);

bool async_mqtt_client_is_connected(struct async_mqtt_client_t *self_p);

#endif
