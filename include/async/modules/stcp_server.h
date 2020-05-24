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
 * The secure TCP server is a TCP server with optional encryption
 * using SSL/TLS.
 */

#ifndef ASYNC_STCP_SERVER_H
#define ASYNC_STCP_SERVER_H

#include "async/core.h"
#include "async/modules/ssl.h"

struct async_stcp_server_t;
struct async_stcp_server_client_t;

typedef void (*async_stcp_server_client_connected_t)(
    struct async_stcp_server_client_t *self_p);

typedef void (*async_stcp_server_client_disconnected_t)(
    struct async_stcp_server_client_t *self_p);

typedef void (*async_stcp_server_client_input_t)(
    struct async_stcp_server_client_t *self_p);

struct async_stcp_server_t {
    struct async_tcp_server_t tcp;
    struct {
        struct async_ssl_context_t *context_p;
        struct async_ssl_connection_t connection;
    } ssl;
    async_stcp_server_client_connected_t on_connected;
    async_stcp_server_client_disconnected_t on_disconnected;
    async_stcp_server_client_input_t on_input;
    struct async_t *async_p;
};

struct async_stcp_server_client_t {
    struct async_tcp_server_client_t tcp;
};

/**
 * Initialize given secure TCP server object.
 */
void async_stcp_server_init(struct async_stcp_server_t *self_p,
                            struct async_ssl_context_t *ssl_context_p,
                            async_stcp_server_client_connected_t on_connected,
                            async_stcp_server_client_disconnected_t on_disconnected,
                            async_stcp_server_client_input_t on_input,
                            struct async_t *async_p);

/**
 * Add given client to given server.
 */
void async_stcp_server_add_client(struct async_stcp_server_t *self_p,
                                  struct async_stcp_server_client_t *client_p);

/**
 * Start listening for clients.
 */
void async_stcp_server_start(struct async_stcp_server_t *self_p);

/**
 * Disconnect any connected clients and stop listening for clients.
 */
void async_stcp_server_stop(struct async_stcp_server_t *self_p);

/**
 * Write size bytes to the remote host.
 */
void async_stcp_server_client_write(struct async_stcp_server_client_t *self_p,
                                    const void *buf_p,
                                    size_t size);

/**
 * Read up to size bytes from the remote host. Returns the number of
 * read bytes (0..size).
 */
size_t async_stcp_server_client_read(struct async_stcp_server_client_t *self_p,
                                     void *buf_p,
                                     size_t size);

/**
 * Disconnect from the remote host.
 */
void async_stcp_server_client_disconnect(struct async_stcp_server_client_t *self_p);

#endif
