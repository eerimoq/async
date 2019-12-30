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

#ifndef ASYNC_SSL_H
#define ASYNC_SSL_H

#include <stdlib.h>
#include "mbedtls/ssl.h"

enum async_ssl_protocol_t {
    async_ssl_protocol_tls_v1_0_t
};

enum async_ssl_verify_mode_t {
    async_ssl_verify_mode_cert_none_t = 0,
    async_ssl_verify_mode_cert_required_t = 2
};

struct async_ssl_connection_t;

typedef void (*async_ssl_connection_on_connected_t)(
    struct async_ssl_connection_t *connection_p,
    int res);

typedef void (*async_ssl_connection_on_disconnected_t)(
    struct async_ssl_connection_t *connection_p);

typedef void (*async_ssl_connection_on_input_t)(
    struct async_ssl_connection_t *connection_p);

typedef ssize_t (*async_ssl_connection_transport_read_t)(
    struct async_ssl_connection_t *connection_p,
    void *buf_p,
    size_t size);

typedef void (*async_ssl_connection_transport_write_t)(
    struct async_ssl_connection_t *connection_p,
    const void *buf_p,
    size_t size);

struct async_ssl_context_t {
    enum async_ssl_protocol_t protocol;
    mbedtls_ssl_config conf;
    int server_side;
    int verify_mode;
};

struct async_ssl_connection_t {
    struct async_ssl_context_t *context_p;
    mbedtls_ssl_context ssl;
    struct {
        bool complete;
        int res;
    } handshake;
    async_ssl_connection_on_connected_t on_connected;
    async_ssl_connection_on_disconnected_t on_disconnected;
    async_ssl_connection_on_input_t on_input;
    struct {
        async_ssl_connection_transport_read_t read;
        async_ssl_connection_transport_write_t write;
    } transport;
    struct async_t *async_p;
};

/**
 * Initialize the module. This function must be called before any
 * other function in this module.
 */
int async_ssl_module_init(void);

/**
 * Initialize given SSL context. A SSL context contains settings that
 * lives longer than a socket.
 */
int async_ssl_context_init(struct async_ssl_context_t *self_p,
                           enum async_ssl_protocol_t protocol);

/**
 * Destroy given SSL context. The context may not be used after it has
 * been destroyed.
 */
int async_ssl_context_destroy(struct async_ssl_context_t *self_p);

/**
 * Load given certificate chain into given context.
 */
int async_ssl_context_load_cert_chain(struct async_ssl_context_t *self_p,
                                      const char *cert_p,
                                      const char *key_p);

/**
 * Load a set of "certification authority" (CA) certificates used to
 * validate other peers’ certificates when ``verify_mode`` is other
 * than `async_ssl_verify_mode_cert_none_t`.
 */
int async_ssl_context_load_verify_location(struct async_ssl_context_t *self_p,
                                           const char *ca_certs_p);

/**
 * Whether to try to verify other peers’ certificates.
 *
 * Load CA certificates with `async_ssl_context_load_verify_location()`.
 */
int async_ssl_context_set_verify_mode(struct async_ssl_context_t *self_p,
                                      enum async_ssl_verify_mode_t mode);

/**
 * Initialize given SSL connection with given socket SSL context and
 * callbacks. Performs the SSL handshake. Transport callbacks often
 * read and write data over a TCP connection.
 */
int async_ssl_connection_open(
    struct async_ssl_connection_t *self_p,
    struct async_ssl_context_t *context_p,
    async_ssl_connection_on_connected_t on_connected,
    async_ssl_connection_on_disconnected_t on_disconnected,
    async_ssl_connection_on_input_t on_input,
    async_ssl_connection_transport_read_t transport_read,
    async_ssl_connection_transport_write_t transport_write,
    struct async_t *async_p);

/**
 * Close given SSL connection.
 */
void async_ssl_connection_close(struct async_ssl_connection_t *self_p);

/**
 * Read data from given SSL connection. Returns the number of read
 * bytes or negative error code.
 */
size_t async_ssl_connection_read(struct async_ssl_connection_t *self_p,
                                 void *buf_p,
                                 size_t size);

/**
 * Write data to given SSL connection. Returns the number of written
 * bytes or negative error code.
 */
void async_ssl_connection_write(struct async_ssl_connection_t *self_p,
                                const void *buf_p,
                                size_t size);

/**
 * Called when transport input is available.
 */
void async_ssl_connection_on_transport_input(struct async_ssl_connection_t *self_p);

#endif
