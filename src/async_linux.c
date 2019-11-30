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

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include "async/linux.h"

static ssize_t stdin_read(struct async_channel_t *self_p,
                          void *buf_p,
                          size_t size)
{
    (void)self_p;

    ssize_t res;
    
    res = read(fileno(stdin), buf_p, size);

    if (res == -1) {
        if (errno == EAGAIN) {
            res = 0;
        } else {
            exit(1);
        }
    }

    return (res);
}

static ssize_t stdin_write(struct async_channel_t *self_p,
                           const void *buf_p,
                           size_t size)
{
    (void)self_p;

    return (write(fileno(stdout), buf_p, size));
}

int async_linux_create_periodic_timer(struct async_t *async_p)
{
    int timer_fd;
    struct itimerspec timeout;

    timer_fd = timerfd_create(CLOCK_REALTIME, 0);

    if (timer_fd == -1) {
        return (timer_fd);
    }

    timeout.it_value.tv_sec = 0;
    timeout.it_value.tv_nsec = async_p->tick_in_ms * 1000000;
    timeout.it_interval.tv_sec= 0;
    timeout.it_interval.tv_nsec = async_p->tick_in_ms * 1000000;
    timerfd_settime(timer_fd, 0, &timeout, NULL);

    return (timer_fd);
}

void async_linux_handle_timeout(struct async_t *async_p,
                                int timer_fd)
{
    uint64_t value;
    ssize_t res;

    res = read(timer_fd, &value, sizeof(value));

    if (res != sizeof(value)) {
        perror("read timer");
        exit(1);
    }

    async_tick(async_p);
}

void async_linux_channel_stdin_init(struct async_channel_t *channel_p,
                                    struct async_t *async_p)
{
    async_channel_init(channel_p,
                       NULL,
                       NULL,
                       stdin_read,
                       stdin_write,
                       async_p);
}

void async_linux_channel_stdin_handle(struct async_channel_t *channel_p)
{
    async_channel_input(channel_p);
}
