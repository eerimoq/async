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

#include <dbg.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "async.h"
#include "asyncio.h"

#define MESSAGE_TYPE_TIMEOUT                             1
#define MESSAGE_TYPE_TCP_CONNECT                         2
#define MESSAGE_TYPE_TCP_CONNECT_COMPLETE                3
#define MESSAGE_TYPE_TCP_DISCONNECT                      4

struct message_header_t {
    int type;
    int size;
    async_func_t func;
    void *obj_p;
};

struct message_tcp_connect_t {
    struct message_header_t header;
    const char *host_p;
    int port;
};

struct message_tcp_disconnect_t {
    struct message_header_t header;
    int sock;
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

static void io_handle_timeout(struct asyncio_t *self_p,
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

static void io_handle_tcp_connect(struct asyncio_t *self_p,
                                  struct message_header_t *header_p)
{
    (void)self_p;
    (void)header_p;

    dbg("");

#if 0
    socket(); SOCK_NONBLOCK
    connect();

    struct message_tcp_connect_complete_t message;

    message.header.type = MESSAGE_TYPE_TCP_CONNECT_COMPLETE;
    message.sock = sock;
    write(self_p->async_fd, &message, sizeof(message));
#endif
}

static void io_handle_tcp_disconnect(struct asyncio_t *self_p,
                                     struct message_header_t *header_p)
{
    (void)self_p;
    (void)header_p;

    dbg("");
}

static void io_handle_async(struct asyncio_t *self_p)
{
    ssize_t res;
    struct message_header_t header;

    res = read(self_p->io_fd, &header, sizeof(header));

    if (res != sizeof(header)) {
        return;
    }

    switch (header.type) {

    case MESSAGE_TYPE_TCP_CONNECT:
        io_handle_tcp_connect(self_p, &header);
        break;

    case MESSAGE_TYPE_TCP_DISCONNECT:
        io_handle_tcp_disconnect(self_p, &header);
        break;

    default:
        break;
    }
}

static void *io_main(struct asyncio_t *self_p)
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

    timer_fd = create_periodic_timer(&self_p->async);

    if (timer_fd == -1) {
        return (NULL);
    }

    event.events = EPOLLIN;
    event.data.fd = timer_fd;
    res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event);

    if (res == -1) {
        return (NULL);
    }

    event.events = EPOLLIN;
    event.data.fd = self_p->io_fd;
    res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, self_p->io_fd, &event);

    if (res == -1) {
        return (NULL);
    }

    while (true) {
        nfds = epoll_wait(epoll_fd, &event, 1, -1);

        if (nfds == 1) {
            if (event.data.fd == timer_fd) {
                io_handle_timeout(self_p, timer_fd);
            } else if (event.data.fd == self_p->io_fd) {
                io_handle_async(self_p);
            }
        }
    }

    return (NULL);
}

static void async_handle_timeout(struct asyncio_t *self_p)
{
    async_tick(&self_p->async);
}

static void async_handle_tcp_connect_complete(struct asyncio_t *self_p,
                                              struct message_header_t *header_p)
{
    //asyncio_tcp_set_sock(message.tcp_p, message.sock);
    async_call(&self_p->async, header_p->func, header_p->obj_p);
}

static void *async_main(struct asyncio_t *self_p)
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

        case MESSAGE_TYPE_TCP_CONNECT_COMPLETE:
            async_handle_tcp_connect_complete(self_p, &header);
            break;

        default:
            dbg(header.type);
            break;
        }

        async_process(&self_p->async);
    }

    return (NULL);
}

void asyncio_init(struct asyncio_t *self_p)
{
    int sockets[2];
    int res;

    res = socketpair(AF_UNIX, SOCK_STREAM, 0, &sockets[0]);

    if (res != 0) {
        return;
    }

    self_p->io_fd = sockets[0];
    self_p->async_fd = sockets[1];
    async_init(&self_p->async, 100);

    pthread_create(&self_p->io_pthread,
                   NULL,
                   (void *(*)(void *))io_main,
                   self_p);
    pthread_create(&self_p->async_pthread,
                   NULL,
                   (void *(*)(void *))async_main,
                   self_p);
}

void asyncio_run_forever(struct asyncio_t *self_p)
{
    pthread_join(self_p->io_pthread, NULL);
    pthread_join(self_p->async_pthread, NULL);
}

void asyncio_write(struct asyncio_t *self_p, const void *buf_p, size_t size)
{
    ssize_t res;
    dbg("");
    res = write(self_p->async_fd, buf_p, size);

    if (res != (ssize_t)size) {
        exit(1);
    }
}

void asyncio_tcp_connect_write(struct asyncio_t *self_p,
                               const char *host_p,
                               int port)
{
    struct message_tcp_connect_t message;

    message.header.type = MESSAGE_TYPE_TCP_CONNECT;
    message.header.size = (sizeof(message) - sizeof(message.header));
    message.host_p = strdup(host_p);
    message.port = port;
    asyncio_write(self_p, &message, sizeof(message));
}

void asyncio_tcp_disconnect_write(struct asyncio_t *self_p, int sock)
{
    struct message_tcp_disconnect_t message;

    message.header.type = MESSAGE_TYPE_TCP_DISCONNECT;
    message.header.size = (sizeof(message) - sizeof(message.header));
    message.sock = sock;
    asyncio_write(self_p, &message, sizeof(message));
}
