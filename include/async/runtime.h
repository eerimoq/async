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

#ifndef ASYNC_RUNTIME_H
#define ASYNC_RUNTIME_H

#include "async/core.h"
#include "async/tcp_client.h"

/**
 * Runtime callbacks.
 */
struct async_tcp_client_t;

typedef void (*async_runtime_run_forever_t)(struct async_t *self_p);

typedef void (*async_runtime_tcp_client_init_t)(
    struct async_tcp_client_t *self_p,
    async_func_t on_connect_complete,
    async_func_t on_disconnected,
    async_func_t on_data,
    void *obj_p,
    struct async_t *async_p);

typedef void (*async_runtime_tcp_client_connect_t)(
    struct async_tcp_client_t *self_p,
    const char *host_p,
    int port);

typedef void (*async_runtime_tcp_client_disconnect_t)(
    struct async_tcp_client_t *self_p);

typedef bool (*async_runtime_tcp_client_is_connected_t)(
    struct async_tcp_client_t *self_p);

typedef ssize_t (*async_runtime_tcp_client_write_t)(
    struct async_tcp_client_t *self_p,
    const void *buf_p,
    size_t size);

typedef size_t (*async_runtime_tcp_client_read_t)(
    struct async_tcp_client_t *self_p,
    void *buf_p,
    size_t size);

struct async_runtime_t {
    struct async_t *async_p;
    async_runtime_run_forever_t run_forever;
    struct {
        async_runtime_tcp_client_init_t init;
        async_runtime_tcp_client_connect_t connect;
        async_runtime_tcp_client_disconnect_t disconnect;
        async_runtime_tcp_client_is_connected_t is_connected;
        async_runtime_tcp_client_write_t write;
        async_runtime_tcp_client_read_t read;
    } tcp_client;
};

/**
 * Create a runtime.
 */
struct async_runtime_t *async_create_runtime(void);

#endif
