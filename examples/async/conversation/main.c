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
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include "async.h"
#include "async/linux.h"
#include "bob.h"

static void fatal_perror(const char *message_p)
{
    perror(message_p);
    exit(1);
}

static int init_periodic_timer(struct async_t *async_p,
                               int epoll_fd)
{
    int res;
    int timer_fd;
    struct epoll_event event;

    timer_fd = async_linux_create_periodic_timer(async_p);

    if (timer_fd == -1) {
        fatal_perror("async_linux_create_periodic_timer");
    }

    event.events = EPOLLIN;
    event.data.fd = timer_fd;
    res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event);

    if (res == -1) {
        fatal_perror("epoll_ctl timer");
    }

    return (timer_fd);
}

static void init_stdin(int epoll_fd)
{
    int res;
    struct epoll_event event;

    res = fcntl(fileno(stdin),
                F_SETFL,
                fcntl(fileno(stdin), F_GETFL) | O_NONBLOCK);

    if (res == -1) {
        fatal_perror("fcntl");
    }

    event.events = EPOLLIN;
    event.data.fd = fileno(stdin);
    res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fileno(stdin), &event);

    if (res == -1) {
        fatal_perror("epoll_ctl stdin");
    }
}

int main()
{
    struct async_t async;
    int timer_fd;
    int epoll_fd;
    struct epoll_event event;
    int nfds;
    struct bob_t bob;
    struct async_channel_t channel;

    async_init(&async, 100);
    async_linux_stream_stdin_init(&channel, &async);
    bob_init(&bob, &channel, &async);

    epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        fatal_perror("epoll_create1");
    }

    timer_fd = init_periodic_timer(&async, epoll_fd);
    init_stdin(epoll_fd);

    while (true) {
        nfds = epoll_wait(epoll_fd, &event, 1, -1);

        if (nfds == 1) {
            if (event.data.fd == timer_fd) {
                async_linux_handle_timeout(&async, timer_fd);
            } else if (event.data.fd == fileno(stdin)) {
                async_linux_stream_stdin_handle(&channel);
            }
        }

        async_process(&async);
    }

    return (1);
}
