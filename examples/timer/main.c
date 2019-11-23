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
#include "async_linux.h"

struct timers_t {
    struct async_timer_t timer_1;
    struct async_timer_t timer_2;
    struct async_timer_t timer_3;
};

static void on_timeout_1(struct timers_t *self_p)
{
    if (!async_timer_is_stopped(&self_p->timer_1)) {
        printf("Timer 1 expired.\n");
    }
}

static void on_timeout_2(void)
{
    printf("Timer 2 expired.\n");
}

static void on_timeout_3(struct timers_t *self_p)
{
    printf("Timer 3 expired. Stopping timer 1.\n");
    async_timer_stop(&self_p->timer_1);
}

int main()
{
    struct async_t async;
    struct timers_t timers;
    int timer_fd;
    ssize_t res;
    uint64_t value;

    async_init(&async, 100);
    async_timer_init(&timers.timer_1,
                     (async_func_t)on_timeout_1,
                     &timers,
                     ASYNC_TIMER_PERIODIC,
                     &async);
    async_timer_init(&timers.timer_2,
                     (async_func_t)on_timeout_2,
                     &timers,
                     ASYNC_TIMER_PERIODIC,
                     &async);
    async_timer_init(&timers.timer_3,
                     (async_func_t)on_timeout_3,
                     &timers,
                     0,
                     &async);
    async_timer_start(&timers.timer_1, 1000);
    async_timer_start(&timers.timer_2, 3000);
    async_timer_start(&timers.timer_3, 5000);

    timer_fd = async_linux_create_periodic_timer(&async);

    while (true) {
        res = read(timer_fd, &value, sizeof(value));

        if (res != sizeof(value)) {
            break;
        }

        async_tick(&async);
        async_process(&async);
    }

    return (1);
}
