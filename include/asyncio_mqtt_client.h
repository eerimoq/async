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

#ifndef ASYNCIO_MQTT_CLIENT_H
#define ASYNCIO_MQTT_CLIENT_H

#include "asyncio.h"

struct asyncio_mqtt_client_message_t {
    const char *topic_p;
    void *data_p;
    size_t size;
};

struct asyncio_mqtt_client_t {
    const char *host_p;
    int port;
    async_func_t on_connected;
    async_func_t on_disconnected;
    async_func_t on_publish;
    void *obj_p;
    struct asyncio_t *asyncio_p;
    const char *client_id_p;
    int keep_alive_s;
    int response_timeout;
    int session_expiry_interval;
    bool connected;
};

void asyncio_mqtt_client_init(struct asyncio_mqtt_client_t *self_p,
                              const char *host_p,
                              int port,
                              async_func_t on_connected,
                              async_func_t on_disconnected,
                              async_func_t on_publish,
                              void *obj_p,
                              struct asyncio_t *asyncio_p);

void asyncio_mqtt_client_set_client_id(struct asyncio_mqtt_client_t *self_p,
                                       const char *client_id_p);

void asyncio_mqtt_client_set_response_timeout(struct asyncio_mqtt_client_t *self_p,
                                              int response_timeout);

void asyncio_mqtt_client_set_session_expiry_interval(
    struct asyncio_mqtt_client_t *self_p,
    int session_expiry_interval);

void asyncio_mqtt_client_start(struct asyncio_mqtt_client_t *self_p);

void asyncio_mqtt_client_stop(struct asyncio_mqtt_client_t *self_p);

void asyncio_mqtt_client_subscribe(struct asyncio_mqtt_client_t *self_p,
                                   const char *topic_p);

void asyncio_mqtt_client_unsubscribe(struct asyncio_mqtt_client_t *self_p,
                                     const char *topic_p);

void asyncio_mqtt_client_publish(struct asyncio_mqtt_client_t *self_p,
                                 const char *topic_p,
                                 const void *buf_p,
                                 size_t size);

void asyncio_mqtt_client_message_free(struct asyncio_mqtt_client_message_t *self_p);

#endif