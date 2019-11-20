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
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "async.h"
#include "async_linux.h"

#define MESSAGE_TYPE_TIMEOUT 1
#define MESSAGE_TYPE_SEND    2

struct message_header_t {
    int type;
    int size;
    struct async_uid_t *uid_p;
    struct async_task_t *receiver_p;
};

static int create_periodic_timer(struct async_t *async_p)
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

static void io_handle_timeout(struct async_linux_t *self_p,
                              int timer_fd)
{
    uint64_t value;
    ssize_t res;
    struct message_header_t header;

    res = read(timer_fd, &value, sizeof(value));

    if (res != sizeof(value)) {
        return;
    }

    header.type = MESSAGE_TYPE_TIMEOUT;
    header.size = 0;
    res = write(self_p->io_fd, &header, sizeof(header));

    if (res != sizeof(header)) {
        return;
    }
}

static void *io_main(struct async_linux_t *self_p)
{
    ssize_t res;
    int timer_fd;
    int nfds;
    int epoll_fd;
    struct epoll_event event;

    epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        return (NULL);
    }

    timer_fd = create_periodic_timer(self_p->async_p);

    if (timer_fd == -1) {
        return (NULL);
    }

    event.events = EPOLLIN;
    event.data.fd = timer_fd;

    res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event);

    if (res == -1) {
        return (NULL);
    }

    while (true) {
        nfds = epoll_wait(epoll_fd, &event, 1, -1);

        if (nfds == 1) {
            if (event.data.fd == timer_fd) {
                io_handle_timeout(self_p, timer_fd);
            } else if (event.data.fd == self_p->async_fd) {
                /* Manage sockets. */
            }
        }
    }

    return (NULL);
}

static void async_handle_timeout(struct async_linux_t *self_p)
{
    async_tick(self_p->async_p);
}

static void async_handle_send(struct async_linux_t *self_p,
                              struct message_header_t *header_p)
{
    void *message_p;
    ssize_t res;

    message_p = async_message_alloc(self_p->async_p,
                                    header_p->uid_p,
                                    header_p->size);
    res = read(self_p->async_fd, message_p, header_p->size);

    if (res == header_p->size) {
        async_send(header_p->receiver_p, message_p);
    }
}

static void *async_main(struct async_linux_t *self_p)
{
    struct message_header_t header;
    ssize_t res;

    while (true) {
        res = read(self_p->async_fd, &header, sizeof(header));

        if (res != sizeof(header)) {
            continue;
        }

        switch (header.type) {

        case MESSAGE_TYPE_TIMEOUT:
            async_handle_timeout(self_p);
            break;

        case MESSAGE_TYPE_SEND:
            async_handle_send(self_p, &header);
            break;

        default:
            break;
        }

        async_process(self_p->async_p);
    }

    return (NULL);
}

void async_linux_create(struct async_linux_t *self_p,
                        struct async_t *async_p)
{
    int sockets[2];
    int res;

    res = socketpair(AF_UNIX, SOCK_STREAM, 0, &sockets[0]);

    if (res != 0) {
        return;
    }

    self_p->io_fd = sockets[0];
    self_p->async_fd = sockets[1];
    self_p->async_p = async_p;

    pthread_create(&self_p->io_pthread,
                   NULL,
                   (void *(*)(void *))io_main,
                   self_p);
    pthread_create(&self_p->async_pthread,
                   NULL,
                   (void *(*)(void *))async_main,
                   self_p);
}

void async_linux_join(struct async_linux_t *self_p)
{
    pthread_join(self_p->io_pthread, NULL);
    pthread_join(self_p->async_pthread, NULL);
}
