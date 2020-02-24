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

#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include "async.h"
#include "async/modules/ssl.h"

#include "mbedtls/config.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl_cookie.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/timing.h"

struct module_t {
    bool initialized;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_x509_crt cert;
    mbedtls_pk_context key;
    mbedtls_x509_crt ca_certs;
};

static struct module_t module;

static void on_input_wrapper(struct async_ssl_connection_t *self_p)
{
    self_p->input_call_outstanding = false;
    self_p->on_input(self_p);
}

static int ssl_send(struct async_ssl_connection_t *self_p,
                    const unsigned char *buf_p,
                    size_t size)
{
    self_p->transport.write(self_p, buf_p, size);

    return (size);
}

static int ssl_recv(struct async_ssl_connection_t *self_p,
                    unsigned char *buf_p,
                    size_t size)
{
    ssize_t res;

    res = self_p->transport.read(self_p, buf_p, size);

    if (res == 0) {
        res = MBEDTLS_ERR_SSL_WANT_READ;
    }

    return (res);
}

static void on_handshake_complete(struct async_ssl_connection_t *self_p)
{
    self_p->on_connected(self_p, self_p->handshake.res);
}

static void handshake(struct async_ssl_connection_t *self_p)
{
    int res;
    char message[128];

    res = mbedtls_ssl_handshake(&self_p->ssl);

    if (res != MBEDTLS_ERR_SSL_WANT_READ) {
        if (res != 0) {
            mbedtls_strerror(res, &message[0], sizeof(message));
            printf("Mbed TLS error: %s\n", &message[0]);
            res = -1;
        }

        self_p->handshake.complete = true;
        self_p->handshake.res = res;
        async_call(self_p->async_p, (async_func_t)on_handshake_complete, self_p);
    }
}

int async_ssl_module_init()
{
    int res;

    /* Return immediately if the module is already initialized. */
    if (module.initialized) {
        return (0);
    }

    module.initialized = true;

    mbedtls_entropy_init(&module.entropy);
    mbedtls_ctr_drbg_init(&module.ctr_drbg);

    /* Random generator seed initialization. */
    res = mbedtls_ctr_drbg_seed(&module.ctr_drbg,
                                mbedtls_entropy_func,
                                &module.entropy,
                                NULL,
                                0);

    return (res == 0 ? 0 : -1);
}

int async_ssl_context_init(struct async_ssl_context_t *self_p,
                           enum async_ssl_protocol_t protocol)
{
    self_p->protocol = protocol;
    mbedtls_ssl_config_init(&self_p->conf);
    mbedtls_ssl_conf_rng(&self_p->conf,
                         mbedtls_ctr_drbg_random,
                         &module.ctr_drbg);

    self_p->server_side = -1;
    self_p->verify_mode = -1;

    return (0);
}

int async_ssl_context_destroy(struct async_ssl_context_t *self_p)
{
    (void)self_p;

    return (0);
}

int async_ssl_context_load_cert_chain(struct async_ssl_context_t *self_p,
                                      const char *cert_p,
                                      const char *key_p)
{
    mbedtls_x509_crt_init(&module.cert);
    mbedtls_pk_init(&module.key);

    /* Parse the certificate. */
    if (mbedtls_x509_crt_parse(&module.cert,
                               (unsigned char *)cert_p,
                               strlen(cert_p) + 1) != 0) {
        return (-1);
    }

    /* Parse the key if present. */
    if (key_p != NULL) {
        if (mbedtls_pk_parse_key(&module.key,
                                 (unsigned char *)key_p,
                                 strlen(key_p) + 1,
                                 NULL,
                                 0) != 0) {
            return (-1);
        }
    }

    if (mbedtls_ssl_conf_own_cert(&self_p->conf,
                                  &module.cert,
                                  &module.key) != 0) {
        return (-1);
    }

    return (0);
}

int async_ssl_context_load_verify_location(struct async_ssl_context_t *self_p,
                                           const char *ca_certs_p)
{
    /* Parse the CA certificate(s). */
    if (mbedtls_x509_crt_parse(&module.ca_certs,
                               (unsigned char *)ca_certs_p,
                               strlen(ca_certs_p) + 1) != 0) {
        return (-1);
    }

    mbedtls_ssl_conf_ca_chain(&self_p->conf,
                              &module.ca_certs,
                              NULL);

    return (0);
}

int async_ssl_context_set_verify_mode(struct async_ssl_context_t *self_p,
                                      enum async_ssl_verify_mode_t mode)
{
    self_p->verify_mode = mode;

    return (0);
}

int async_ssl_connection_open(
    struct async_ssl_connection_t *self_p,
    struct async_ssl_context_t *context_p,
    async_ssl_connection_on_connected_t on_connected,
    async_ssl_connection_on_disconnected_t on_disconnected,
    async_ssl_connection_on_input_t on_input,
    async_ssl_connection_transport_read_t transport_read,
    async_ssl_connection_transport_write_t transport_write,
    struct async_t *async_p)
{
    int res;
    int server_side;
    const char *server_hostname_p;

    self_p->context_p = context_p;
    self_p->handshake.complete = false;
    self_p->input_call_outstanding = false;
    self_p->on_connected = on_connected;
    self_p->on_disconnected = on_disconnected;
    self_p->on_input = on_input;
    self_p->transport.read = transport_read;
    self_p->transport.write = transport_write;
    self_p->async_p = async_p;

    server_side = 0; // (flags & ASYNC_SSL_SOCKET_SERVER_SIDE);
    server_hostname_p = NULL;

    /* This implementation can only handle a single socket mode,
       server or client, in a context. Which mode to configure is
       first known in this function, so the context cannot be
       configured earlier. */
    if (context_p->server_side == -1) {
        if (mbedtls_ssl_config_defaults(&context_p->conf,
                                        server_side,
                                        MBEDTLS_SSL_TRANSPORT_STREAM,
                                        MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
            return (-1);
        }

        /* Use mbedTLS default values (none for servers, required for
           clients. */
        if (context_p->verify_mode != -1) {
            mbedtls_ssl_conf_authmode(&context_p->conf, context_p->verify_mode);
        }

        context_p->server_side = server_side;
    } else if (context_p->server_side != server_side) {
        return (-1);
    }

    /* Inilialize the SSL session. */
    mbedtls_ssl_init(&self_p->ssl);

    res = mbedtls_ssl_setup(&self_p->ssl, &self_p->context_p->conf);

    if (res != 0) {
        return (-1);
    }

    mbedtls_ssl_set_bio(&self_p->ssl,
                        self_p,
                        (int (*)(void *, const unsigned char *, size_t))ssl_send,
                        (int (*)(void *, unsigned char *, size_t))ssl_recv,
                        NULL);

    /* Server hostname for client side sockets. */
    if (server_side == 0) {
        if (server_hostname_p != NULL) {
            if (mbedtls_ssl_set_hostname(&self_p->ssl,
                                         server_hostname_p) != 0) {
                return (-1);
            }
        }
    }

    /* Start the handshake with the remote peer. */
    handshake(self_p);

    return (0);
}

void async_ssl_connection_close(struct async_ssl_connection_t *self_p)
{
    mbedtls_ssl_close_notify(&self_p->ssl);
    mbedtls_ssl_free(&self_p->ssl);
}

size_t async_ssl_connection_read(struct async_ssl_connection_t *self_p,
                                 void *buf_p,
                                 size_t size)
{
    ssize_t res;

    res = mbedtls_ssl_read(&self_p->ssl, buf_p, size);

    if (res == MBEDTLS_ERR_SSL_WANT_READ) {
        res = 0;
    } else if (res == MBEDTLS_ERR_SSL_CLIENT_RECONNECT) {
        res = 0;
    }

    if (!self_p->input_call_outstanding) {
        if (mbedtls_ssl_get_bytes_avail(&self_p->ssl) > 0) {
            self_p->input_call_outstanding = true;
            async_call(self_p->async_p, (async_func_t)on_input_wrapper, self_p);
        }
    }

    return (res);
}

void async_ssl_connection_write(struct async_ssl_connection_t *self_p,
                                const void *buf_p,
                                size_t size)
{
    mbedtls_ssl_write(&self_p->ssl, buf_p, size);
}

void async_ssl_connection_on_transport_input(struct async_ssl_connection_t *self_p)
{
    if (self_p->handshake.complete) {
        self_p->on_input(self_p);
    } else {
        /* Continue the handshake. */
        handshake(self_p);
    }
}
