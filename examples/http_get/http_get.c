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
#include "async.h"
#include "http_get.h"

static void do_get(struct http_get_t *self_p)
{
    printf("Getting 'http://localhost:8080/main.c'...\n");
    async_tcp_client_connect(&self_p->tcp, "localhost", 8080);
}

static void on_connected(struct async_tcp_client_t *tcp_p, int res)
{
    struct http_get_t *self_p;
    static const char request[] = (
        "GET /main.c HTTP/1.1\r\n"
        "\r\n");

    self_p = async_container_of(tcp_p, typeof(*self_p), tcp);

    if (res == 0) {
        printf("--------------------- HTTP GET BEGIN ---------------------\n");
        async_tcp_client_write(tcp_p, &request[0], sizeof(request) - 1);
    } else {
        printf("Connect failed.\n");
    }
}

static void on_disconnected(struct async_tcp_client_t *tcp_p)
{
    (void)tcp_p;

    printf("---------------------- HTTP GET END ----------------------\n");
}

static void on_input(struct async_tcp_client_t *tcp_p)
{
    char buf[64];
    ssize_t size;
    struct http_get_t *self_p;

    self_p = async_container_of(tcp_p, typeof(*self_p), tcp);
    size = async_tcp_client_read(&self_p->tcp, &buf[0], sizeof(buf));

    fwrite(&buf[0], 1, size, stdout);
}

void http_get_init(struct http_get_t *self_p, struct async_t *async_p)
{
    async_tcp_client_init(&self_p->tcp,
                          on_connected,
                          on_disconnected,
                          on_input,
                          async_p);
    async_call(async_p, (async_func_t)do_get, self_p);
}
