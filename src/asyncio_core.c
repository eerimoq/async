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

#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "async_linux.h"
#include "asyncio.h"
#include "internalio.h"

#define MESSAGE_TYPE_TIMEOUT                             1
#define MESSAGE_TYPE_TCP_CONNECT                         2
#define MESSAGE_TYPE_TCP_CONNECT_COMPLETE                3
#define MESSAGE_TYPE_TCP_DISCONNECT                      4
#define MESSAGE_TYPE_TCP_DATA                            5
#define MESSAGE_TYPE_TCP_DATA_COMPLETE                   6
#define MESSAGE_TYPE_TCP_DISCONNECTED                    7

struct message_connect_t {
    struct asyncio_tcp_t *tcp_p;
    char *host_p;
    int port;
};

struct message_connect_complete_t {
    struct asyncio_tcp_t *tcp_p;
    int sockfd;
};

struct message_disconnect_t {
    struct asyncio_tcp_t *tcp_p;
    int sockfd;
};

struct message_data_complete_t {
    struct asyncio_tcp_t *tcp_p;
    bool closed;
};

static struct asyncio_tcp_t *tcp_p;

static void read_buf(int fd, void *buf_p, size_t size)
{
    ssize_t res;

    res = read(fd, buf_p, size);

    if (res != (ssize_t)size) {
        exit(1);
    }
}

static void write_buf(int fd, const void *buf_p, size_t size)
{
    ssize_t res;

    res = write(fd, buf_p, size);

    if (res != (ssize_t)size) {
        exit(1);
    }
}

static void write_message_type(int fd, int type)
{
    write_buf(fd, &type, sizeof(type));
}

static void write_message(int fd, int type, const void *buf_p, size_t size)
{
    write_message_type(fd, type);
    write_buf(fd, buf_p, size);
}

static void io_handle_timeout(struct asyncio_t *self_p,
                              int timer_fd)
{
    uint64_t value;
    int type;

    read_buf(timer_fd, &value, sizeof(value));
    type = MESSAGE_TYPE_TIMEOUT;
    write_buf(self_p->io_fd, &type, sizeof(type));
}

static void io_handle_tcp_connect(struct asyncio_t *self_p,
                                  int epoll_fd)
{
    struct sockaddr_in addr;
    int sockfd;
    struct message_connect_t req;
    struct message_connect_complete_t rsp;
    int res;
    struct epoll_event event;

    read_buf(self_p->io_fd, &req, sizeof(req));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(req.port);
    inet_aton(req.host_p, (struct in_addr *)&addr.sin_addr.s_addr);
    free(req.host_p);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd != -1) {
        res = connect(sockfd, &addr, sizeof(addr));

        if (res != -1) {
            res = fcntl(sockfd,
                        F_SETFL,
                        fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

            if (res != -1) {
                event.events = EPOLLIN;
                event.data.fd = sockfd;
                res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event);
            }
        }

        if (res == -1) {
            sockfd = -1;
        }
    }

    rsp.tcp_p = req.tcp_p;
    rsp.sockfd = sockfd;
    tcp_p = req.tcp_p;
    write_message(self_p->io_fd,
                  MESSAGE_TYPE_TCP_CONNECT_COMPLETE,
                  &rsp,
                  sizeof(rsp));
}

static void io_handle_tcp_disconnect(struct asyncio_t *self_p)
{
    (void)self_p;
}

static void io_handle_tcp_data_complete(struct asyncio_t *self_p,
                                        int epoll_fd)
{
    int sockfd;
    struct epoll_event event;
    struct message_data_complete_t ind;

    read_buf(self_p->io_fd, &ind, sizeof(ind));

    sockfd = ind.tcp_p->sockfd;

    if (ind.closed) {
        close(sockfd);
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, NULL);
        write_message_type(self_p->io_fd, MESSAGE_TYPE_TCP_DISCONNECTED);
    } else {
        event.events = EPOLLIN;
        event.data.fd = sockfd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
    }
}

static void io_handle_async(struct asyncio_t *self_p,
                            int epoll_fd)
{
    int type;

    read_buf(self_p->io_fd, &type, sizeof(type));

    switch (type) {

    case MESSAGE_TYPE_TCP_CONNECT:
        io_handle_tcp_connect(self_p, epoll_fd);
        break;

    case MESSAGE_TYPE_TCP_DISCONNECT:
        io_handle_tcp_disconnect(self_p);
        break;

    case MESSAGE_TYPE_TCP_DATA_COMPLETE:
        io_handle_tcp_data_complete(self_p, epoll_fd);
        break;

    default:
        break;
    }
}

static void io_handle_socket(struct asyncio_t *self_p, int epoll_fd)
{
    struct epoll_event event;

    event.events = 0;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, tcp_p->sockfd, &event);
    write_message_type(self_p->io_fd, MESSAGE_TYPE_TCP_DATA);
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

    timer_fd = async_linux_create_periodic_timer(&self_p->async);

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
                io_handle_async(self_p, epoll_fd);
            } else {
                io_handle_socket(self_p, epoll_fd);
            }
        }
    }

    return (NULL);
}

static void async_handle_timeout(struct asyncio_t *self_p)
{
    async_tick(&self_p->async);
}

static void async_handle_tcp_connect_complete(struct asyncio_t *self_p)
{
    struct message_connect_complete_t ind;

    read_buf(self_p->async_fd, &ind, sizeof(ind));
    asyncio_tcp_set_sockfd(ind.tcp_p, ind.sockfd);
    async_call(&self_p->async,
               ind.tcp_p->on_connect_complete,
               ind.tcp_p->obj_p);
}

static void async_handle_tcp_data(struct asyncio_t *self_p)
{
    async_call(&self_p->async, tcp_p->on_data, tcp_p->obj_p);
}

static void async_handle_tcp_disconnected(struct asyncio_t *self_p)
{
    asyncio_tcp_set_sockfd(tcp_p, -1);
    async_call(&self_p->async, tcp_p->on_disconnected, tcp_p->obj_p);
}

static void *async_main(struct asyncio_t *self_p)
{
    int type;

    while (true) {
        read_buf(self_p->async_fd, &type, sizeof(type));

        switch (type) {

        case MESSAGE_TYPE_TIMEOUT:
            async_handle_timeout(self_p);
            break;

        case MESSAGE_TYPE_TCP_CONNECT_COMPLETE:
            async_handle_tcp_connect_complete(self_p);
            break;

        case MESSAGE_TYPE_TCP_DATA:
            async_handle_tcp_data(self_p);
            break;

        case MESSAGE_TYPE_TCP_DISCONNECTED:
            async_handle_tcp_disconnected(self_p);
            break;

        default:
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

void asyncio_tcp_connect_write(struct asyncio_tcp_t *self_p,
                               const char *host_p,
                               int port)
{
    struct message_connect_t data;

    data.tcp_p = self_p;
    data.host_p = strdup(host_p);
    data.port = port;
    write_message(self_p->asyncio_p->async_fd,
                  MESSAGE_TYPE_TCP_CONNECT,
                  &data,
                  sizeof(data));
}

void asyncio_tcp_disconnect_write(struct asyncio_tcp_t *self_p)
{
    struct message_disconnect_t data;

    data.sockfd = self_p->sockfd;
    write_message(self_p->asyncio_p->async_fd,
                  MESSAGE_TYPE_TCP_DISCONNECT,
                  &data,
                  sizeof(data));
}

void asyncio_tcp_data_complete_write(struct asyncio_tcp_t *self_p,
                                     bool closed)
{
    struct message_data_complete_t ind;

    ind.tcp_p = self_p;
    ind.closed = closed;
    write_message(self_p->asyncio_p->async_fd,
                  MESSAGE_TYPE_TCP_DATA_COMPLETE,
                  &ind,
                  sizeof(ind));
}
