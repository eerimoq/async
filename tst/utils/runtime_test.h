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

#ifndef RUNTIME_TEST_H
#define RUNTIME_TEST_H

#include "async.h"

void runtime_test_set_async(void *self_p, struct async_t *async_p);

void runtime_test_run_forever(void *self_p);

void runtime_test_tcp_client_init(struct async_tcp_client_t *self_p,
                                  async_tcp_client_connected_t on_connected,
                                  async_tcp_client_disconnected_t on_disconnected,
                                  async_tcp_client_input_t on_input);

void runtime_test_tcp_client_connect(struct async_tcp_client_t *self_p,
                                     const char *host_p,
                                     int port);

void runtime_test_tcp_client_disconnect(struct async_tcp_client_t *self_p);

void runtime_test_tcp_client_write(struct async_tcp_client_t *self_p,
                                   const void *buf_p,
                                   size_t size);

size_t runtime_test_tcp_client_read(struct async_tcp_client_t *self_p,
                                    void *buf_p,
                                    size_t size);

struct async_runtime_t *runtime_test_create(void);

void runtime_test_tcp_server_init(
    struct async_tcp_server_t *self_p,
    const char *host_p,
    int port,
    async_tcp_server_client_connected_t on_connected,
    async_tcp_server_client_disconnected_t on_disconnected,
    async_tcp_server_client_input_t on_input);

void runtime_test_tcp_server_add_client(
    struct async_tcp_server_t *self_p,
    struct async_tcp_server_client_t *client_p);

void runtime_test_tcp_server_start(
    struct async_tcp_server_t *self_p);

void runtime_test_tcp_server_stop(
    struct async_tcp_server_t *self_p);

void runtime_test_tcp_server_client_write(
    struct async_tcp_server_client_t *self_p,
    const void *buf_p,
    size_t size);

size_t runtime_test_tcp_server_client_read(
    struct async_tcp_server_client_t *self_p,
    void *buf_p,
    size_t size);

void runtime_test_tcp_server_client_disconnect(
    struct async_tcp_server_client_t *self_p);

#endif
