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
#include <stdint.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <stdio.h>
#include "async.h"

#define TICK_IN_MS 100

static ASYNC_UID_DEFINE(timeout_1);
static ASYNC_UID_DEFINE(timeout_2);
static ASYNC_UID_DEFINE(timeout_3);

static struct async_t async;
static struct async_task_t task;
static struct async_timer_t timer_1;
static struct async_timer_t timer_2;
static struct async_timer_t timer_3;

static int create_periodic_timer(void)
{
    int timer_fd;
    struct itimerspec timeout;

    timer_fd = timerfd_create(CLOCK_REALTIME, 0);

    if (timer_fd == -1) {
        return (timer_fd);
    }

    timeout.it_value.tv_sec = 0;
    timeout.it_value.tv_nsec = TICK_IN_MS * 1000000;
    timeout.it_interval.tv_sec= 0;
    timeout.it_interval.tv_nsec = TICK_IN_MS * 1000000;
    timerfd_settime(timer_fd, 0, &timeout, NULL);

    return (timer_fd);
}

static void on_message(struct async_task_t *self_p,
                       struct async_uid_t *uid_p,
                       void *message_p)
{
    (void)self_p;
    (void)message_p;

    if (uid_p == &timeout_1) {
        if (!async_timer_is_stopped(&timer_1)) {
            printf("Timer 1 expired.\n");
        }
    } else if (uid_p == &timeout_2) {
        printf("Timer 2 expired.\n");
    } else if (uid_p == &timeout_3) {
        printf("Timer 3 expired. Stopping timer 1.\n");
        async_timer_stop(&timer_1);
    }
}

int main()
{
    uint64_t value;
    ssize_t res;
    int timer_fd;

    timer_fd = create_periodic_timer();

    if (timer_fd == -1) {
        return (1);
    }

    async_init(&async, TICK_IN_MS, NULL, 0);
    async_task_init(&task, &async, on_message);
    async_timer_init(&timer_1, &async, 1000, &timeout_1, &task, ASYNC_TIMER_PERIODIC);
    async_timer_init(&timer_2, &async, 3000, &timeout_2, &task, ASYNC_TIMER_PERIODIC);
    async_timer_init(&timer_3, &async, 5000, &timeout_3, &task, 0);

    printf("Starting timer 1.\n");
    async_timer_start(&timer_1);

    printf("Starting timer 2.\n");
    async_timer_start(&timer_2);

    printf("Starting timer 3.\n");
    async_timer_start(&timer_3);

    while (true) {
        res = read(timer_fd, &value, sizeof(value));

        if (res != sizeof(value)) {
            return (1);
        }

        async_tick(&async);
        async_process(&async);
    }

    return (0);
}
