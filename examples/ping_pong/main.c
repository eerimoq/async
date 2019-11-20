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

static struct async_t async;
static struct async_task_t ping_task;
static struct async_task_t pong_task;

static ASYNC_UID_DEFINE(start);
static ASYNC_UID_DEFINE(ping);
static ASYNC_UID_DEFINE(pong);

static void ping_on_message(struct async_task_t *self_p,
                            struct async_uid_t *uid_p,
                            void *message_p)
{
    (void)self_p;
    (void)message_p;

    void *ping_p;

    if (uid_p == &start) {
        printf("Start received. Sending ping.\n");
        ping_p = async_message_alloc(&async, &ping, 0);
        async_send(&pong_task, ping_p);
    } else if (uid_p == &pong) {
        printf("Pong received. Done.\n");
    }
}

static void pong_on_message(struct async_task_t *self_p,
                            struct async_uid_t *uid_p,
                            void *message_p)
{
    (void)self_p;
    (void)message_p;

    void *pong_p;

    if (uid_p == &ping) {
        printf("Ping received. Sending pong.\n");
        pong_p = async_message_alloc(&async, &pong, 0);
        async_send(&ping_task, pong_p);
    }
}

int main()
{
    void *start_p;

    async_init(&async, 100, NULL, 0);
    async_task_init(&ping_task, &async, ping_on_message);
    async_task_init(&pong_task, &async, pong_on_message);
    start_p = async_message_alloc(&async, &start, 0);
    async_send(&ping_task, start_p);
    async_process(&async);

    return (0);
}
