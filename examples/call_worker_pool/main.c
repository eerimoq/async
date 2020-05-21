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
#include <unistd.h>
#include "async.h"

static void hello(void *obj_p, void *arg_p)
{
    (void)obj_p;
    (void)arg_p;

    printf("Sleeping for 5 seconds...\n");
    sleep(5);
    printf("Woke up!\n");
}

static void on_complete(void *obj_p, void *arg_p)
{
    (void)obj_p;
    (void)arg_p;

    printf("Completed!\n");
}

static void on_timeout()
{
    printf("Timeout!\n");
}

int main()
{
    struct async_t async;
    struct async_timer_t timer;

    async_init(&async);
    async_set_runtime(&async, async_runtime_create());
    async_timer_init(&timer, on_timeout, NULL, 0, 1000, &async);
    async_timer_start(&timer);
    async_call_worker_pool(&async, hello, NULL, NULL, on_complete);
    async_run_forever(&async);

    return (0);
}
