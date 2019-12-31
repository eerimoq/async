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

#include "async.h"

typedef void (*async_mqtt_client_on_publish_t)(void *obj_p,
                                               const char *topic_p,
                                               const uint8_t *buf_p,
                                               size_t size);

typedef void (*async_mqtt_client_on_subscribe_complete_t)(void *obj_p,
                                                          uint16_t transaction_id);

struct async_mqtt_client_packet_t {
    uint8_t buf[256];
    int size;
    int state;
    int offset;
    int type;
    int flags;
};

struct async_mqtt_client_will_t {
    const char *topic_p;
    struct {
        uint8_t *buf_p;
        size_t size;
    } message;
};

struct async_mqtt_client_t {
    const char *host_p;
    int port;
    async_func_t on_connected;
    async_func_t on_disconnected;
    async_mqtt_client_on_publish_t on_publish;
    async_mqtt_client_on_subscribe_complete_t on_subscribe_complete;
    void *obj_p;
    struct async_t *async_p;
    char client_id[64];
    int keep_alive_s;
    struct async_mqtt_client_will_t will;
    bool connected;
    uint16_t next_packet_identifier;
    struct async_stcp_client_t stcp;
    struct async_mqtt_client_packet_t packet;
    struct async_timer_t keep_alive_timer;
    struct async_timer_t reconnect_timer;
};

/**
 * Initialize given MQTT client.
 */
void async_mqtt_client_init(struct async_mqtt_client_t *self_p,
                            const char *host_p,
                            int port,
                            struct async_ssl_context_t *ssl_context_p,
                            async_func_t on_connected,
                            async_func_t on_disconnected,
                            async_mqtt_client_on_publish_t on_publish,
                            void *obj_p,
                            struct async_t *async_p);

/**
 * Set the client identifier. Must be called after
 * async_mqtt_client_init() and before async_mqtt_client_start().
 */
void async_mqtt_client_set_client_id(struct async_mqtt_client_t *self_p,
                                     const char *client_id_p);

/**
 * Set the will topic and message pointers. Must be called after
 * async_mqtt_client_init() and before async_mqtt_client_start().
 */
void async_mqtt_client_set_will(struct async_mqtt_client_t *self_p,
                                const char *topic_p,
                                uint8_t *buf_p,
                                size_t size);

/**
 * Set the on subscribe complete callback. Must be called after
 * async_mqtt_client_init() and before async_mqtt_client_start().
 */
void async_mqtt_client_set_on_subscribe_complete(
    struct async_mqtt_client_t *self_p,
    async_mqtt_client_on_subscribe_complete_t on_subscribe_complete);

/**
 * Start given client. A startd client will try to connect to the
 * broker until successful. `on_connected()` passed to
 * `async_mqtt_client_init()` is called once connected.
 */
void async_mqtt_client_start(struct async_mqtt_client_t *self_p);

/**
 * Stop given client. Diconnect from the broker, if connected.
 */
void async_mqtt_client_stop(struct async_mqtt_client_t *self_p);

/**
 * Subscribe to given topic. Returns the transaction id, passed to
 * `on_subscribe_complete()`, if set, once completed.
 */
uint16_t async_mqtt_client_subscribe(struct async_mqtt_client_t *self_p,
                                     const char *topic_p);

/**
 * Publish to given message on given topic, with quality of service
 * zero (QoS 0).
 */
void async_mqtt_client_publish(struct async_mqtt_client_t *self_p,
                               const char *topic_p,
                               const void *buf_p,
                               size_t size);

#endif
