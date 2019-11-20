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
#include <string.h>
#include "async.h"
#include "async_linux.h"

static int handle = -1;
static struct async_timer_t timer;

static ASYNC_UID_DEFINE(start);
static ASYNC_UID_DEFINE(timeout);

static void handle_start(void)
{
    async_tcp_connect("localhost", 33000);
}

static void handle_timeout()
{
    if (handle != -1) {
        printf("TX: 'Hello!'\n");
        async_tcp_write(handle, "Hello!", 6);
    }
}

static void handle_tcp_connect_response(struct async_tcp_connect_t *message_p)
{
    if (message_p->handle != -1) {
        handle = message_p->handle;
        async_timer_start(&timer);
    } else {
        printf("Connect failed.\n");
    }
}

static void handle_tcp_data(struct async_tcp_data_t *message_p)
{
    if (message_p->length > 0) {
        printf("RX: '");
        fwrite(&message_p->data[0], message_p->length, 1, stdout);
        printf("'\n");
    } else {
        printf("Connection closed by server.\n");
        handle = -1;
    }
}

static void client_on_message(struct async_task_t *self_p,
                              struct async_uid_t *uid_p,
                              void *message_p)
{
    (void)self_p;

    if (uid_p == &start) {
        handle_start();
    } else if (uid_p == &timeout) {
        handle_timeout();
    } else if (uid_p == &async_tcp_message_id_connect) {
        handle_tcp_connect_response(message_p);
    } else if (uid_p == &async_tcp_message_id_data) {
        handle_tcp_data(message_p);
    } else {
        printf("Unexpected message.\n");
    }
}

int main()
{
    struct async_t async;
    struct async_task_t client;
    void *start_p;
    struct async_linux_t async_linux;

    /* Setup. */
    async_init(&async, 100, NULL, 0);
    async_task_init(&client, &async, client_on_message);
    async_timer_init(&timer,
                     &async,
                     1000,
                     &timeout,
                     &client,
                     ASYNC_TIMER_PERIODIC);
    start_p = async_message_alloc(&async, &start, 0);
    async_send(&client, start_p);

    /* Start. */
    async_linux_create(&async_linux, &async);
    async_linux_join(&async_linux);

    return (0);
}
