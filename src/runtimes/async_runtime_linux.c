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

#include <pthread.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <stdio.h>
#include "async.h"
#include "async/utils/linux.h"

#define MESSAGE_TYPE_TIMEOUT                             1
#define MESSAGE_TYPE_TCP_CONNECT                         2
#define MESSAGE_TYPE_TCP_CONNECT_COMPLETE                3
#define MESSAGE_TYPE_TCP_DISCONNECT                      4
#define MESSAGE_TYPE_TCP_DATA                            5
#define MESSAGE_TYPE_TCP_DATA_COMPLETE                   6
#define MESSAGE_TYPE_TCP_DISCONNECTED                    7

struct async_runtime_linux_t {
    int io_fd;
    int async_fd;
    pthread_t io_pthread;
    pthread_t async_pthread;
    struct async_t *async_p;
};

struct message_connect_t {
    struct async_tcp_client_t *tcp_p;
    char *host_p;
    int port;
};

struct message_connect_complete_t {
    struct async_tcp_client_t *tcp_p;
    int sockfd;
};

struct message_disconnect_t {
    struct async_tcp_client_t *tcp_p;
    int sockfd;
};

struct message_data_complete_t {
    struct async_tcp_client_t *tcp_p;
    bool closed;
};

struct tcp_client_t {
    struct {
        async_tcp_client_connected_t func;
        int res;
    } on_connected;
    async_tcp_client_disconnected_t on_disconnected;
    async_tcp_client_input_t on_input;
    int sockfd;
};

static struct async_tcp_client_t *tcp_p;

static struct tcp_client_t *tcp_client(struct async_tcp_client_t *self_p)
{
    return ((struct tcp_client_t *)(self_p->obj_p));
}

static struct async_runtime_linux_t *tcp_runtime(struct async_tcp_client_t *self_p)
{
    return ((struct async_runtime_linux_t *)(self_p->async_p->runtime_p->obj_p));
}

static void async_tcp_client_set_sockfd(struct async_tcp_client_t *self_p, int sockfd)
{
    tcp_client(self_p)->sockfd = sockfd;
}

static void read_buf(int fd, void *buf_p, size_t size)
{
    ssize_t res;

    res = read(fd, buf_p, size);

    if (res != (ssize_t)size) {
        async_utils_linux_fatal_perror("read");
    }
}

static void write_buf(int fd, const void *buf_p, size_t size)
{
    ssize_t res;

    res = write(fd, buf_p, size);

    if (res != (ssize_t)size) {
        async_utils_linux_fatal_perror("write");
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

static void io_handle_timeout(struct async_runtime_linux_t *self_p,
                              int timer_fd)
{
    uint64_t value;
    int type;

    read_buf(timer_fd, &value, sizeof(value));
    type = MESSAGE_TYPE_TIMEOUT;
    write_buf(self_p->io_fd, &type, sizeof(type));
}

static void io_handle_tcp_client_connect(struct async_runtime_linux_t *self_p,
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

static void io_handle_tcp_client_disconnect(struct async_runtime_linux_t *self_p,
                                            int epoll_fd)
{
    (void)self_p;

    close(tcp_client(tcp_p)->sockfd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, tcp_client(tcp_p)->sockfd, NULL);
}

static void io_handle_tcp_client_data_complete(struct async_runtime_linux_t *self_p,
                                               int epoll_fd)
{
    int sockfd;
    struct epoll_event event;
    struct message_data_complete_t ind;

    read_buf(self_p->io_fd, &ind, sizeof(ind));

    sockfd = tcp_client(ind.tcp_p)->sockfd;

    if (ind.closed) {
        close(sockfd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, NULL);
        write_message_type(self_p->io_fd, MESSAGE_TYPE_TCP_DISCONNECTED);
    } else {
        event.events = EPOLLIN;
        event.data.fd = sockfd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
    }
}

static void io_handle_async(struct async_runtime_linux_t *self_p,
                            int epoll_fd)
{
    int type;

    read_buf(self_p->io_fd, &type, sizeof(type));

    switch (type) {

    case MESSAGE_TYPE_TCP_CONNECT:
        io_handle_tcp_client_connect(self_p, epoll_fd);
        break;

    case MESSAGE_TYPE_TCP_DISCONNECT:
        io_handle_tcp_client_disconnect(self_p, epoll_fd);
        break;

    case MESSAGE_TYPE_TCP_DATA_COMPLETE:
        io_handle_tcp_client_data_complete(self_p, epoll_fd);
        break;

    default:
        break;
    }
}

static void io_handle_socket(struct async_runtime_linux_t *self_p,
                             int epoll_fd)
{
    struct epoll_event event;

    event.events = 0;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, tcp_client(tcp_p)->sockfd, &event);
    write_message_type(self_p->io_fd, MESSAGE_TYPE_TCP_DATA);
}

static void *io_main(struct async_runtime_linux_t *self_p)
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

    timer_fd = async_utils_linux_create_periodic_timer(self_p->async_p);

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

static void async_handle_timeout(struct async_runtime_linux_t *self_p)
{
    async_tick(self_p->async_p);
}

static void async_handle_tcp_client_connected(struct async_runtime_linux_t *self_p)
{
    struct message_connect_complete_t ind;

    read_buf(self_p->async_fd, &ind, sizeof(ind));
    async_tcp_client_set_sockfd(ind.tcp_p, ind.sockfd);
    tcp_client(ind.tcp_p)->on_connected.func(ind.tcp_p, ind.sockfd == -1 ? -1 : 0);
}

static void async_handle_tcp_client_input(void)
{
    tcp_client(tcp_p)->on_input(tcp_p);
}

static void async_handle_tcp_client_disconnected(void)
{
    async_tcp_client_set_sockfd(tcp_p, -1);
    tcp_client(tcp_p)->on_disconnected(tcp_p);
}

static void *async_main(struct async_runtime_linux_t *self_p)
{
    int type;

    while (true) {
        read_buf(self_p->async_fd, &type, sizeof(type));

        switch (type) {

        case MESSAGE_TYPE_TIMEOUT:
            async_handle_timeout(self_p);
            break;

        case MESSAGE_TYPE_TCP_CONNECT_COMPLETE:
            async_handle_tcp_client_connected(self_p);
            break;

        case MESSAGE_TYPE_TCP_DATA:
            async_handle_tcp_client_input();
            break;

        case MESSAGE_TYPE_TCP_DISCONNECTED:
            async_handle_tcp_client_disconnected();
            break;

        default:
            break;
        }

        async_process(self_p->async_p);
    }

    return (NULL);
}

static void set_async(struct async_runtime_linux_t *self_p,
                      struct async_t *async_p)
{
    self_p->async_p = async_p;
}

static int spawn(struct async_runtime_linux_t *self_p,
                 async_func_t entry,
                 void *obj_p,
                 async_func_t on_complete)
{
    entry(obj_p);
    async_call(self_p->async_p, on_complete, obj_p);

    return (0);
}

static void run_forever(struct async_runtime_linux_t *self_p)
{
    pthread_create(&self_p->io_pthread,
                   NULL,
                   (void *(*)(void *))io_main,
                   self_p);
    pthread_create(&self_p->async_pthread,
                   NULL,
                   (void *(*)(void *))async_main,
                   self_p);
    pthread_join(self_p->io_pthread, NULL);
    pthread_join(self_p->async_pthread, NULL);
}

void async_tcp_client_connect_write(struct async_tcp_client_t *self_p,
                                    const char *host_p,
                                    int port)
{
    struct message_connect_t data;

    data.tcp_p = self_p;
    data.host_p = strdup(host_p);
    data.port = port;
    write_message(tcp_runtime(self_p)->async_fd,
                  MESSAGE_TYPE_TCP_CONNECT,
                  &data,
                  sizeof(data));
}

void async_tcp_client_disconnect_write(struct async_tcp_client_t *self_p)
{
    struct message_disconnect_t data;

    data.sockfd = tcp_client(self_p)->sockfd;
    write_message(tcp_runtime(self_p)->async_fd,
                  MESSAGE_TYPE_TCP_DISCONNECT,
                  &data,
                  sizeof(data));
}

void async_tcp_client_data_complete_write(struct async_tcp_client_t *self_p,
                                          bool closed)
{
    struct message_data_complete_t ind;

    ind.tcp_p = self_p;
    ind.closed = closed;
    write_message(tcp_runtime(self_p)->async_fd,
                  MESSAGE_TYPE_TCP_DATA_COMPLETE,
                  &ind,
                  sizeof(ind));
}

static void tcp_client_init(struct async_tcp_client_t *self_p,
                            async_tcp_client_connected_t on_connected,
                            async_tcp_client_disconnected_t on_disconnected,
                            async_tcp_client_input_t on_input)
{
    struct tcp_client_t *rself_p;

    rself_p = malloc(sizeof(*rself_p));

    if (rself_p == NULL) {
        return;
    }

    rself_p->on_connected.func = on_connected;
    rself_p->on_disconnected = on_disconnected;
    rself_p->on_input = on_input;
    rself_p->sockfd = -1;
    self_p->obj_p = rself_p;
}

static void tcp_client_connect(struct async_tcp_client_t *self_p,
                               const char *host_p,
                               int port)
{
    async_tcp_client_connect_write(self_p, host_p, port);
}

static void tcp_client_disconnect(struct async_tcp_client_t *self_p)
{
    async_tcp_client_disconnect_write(self_p);
}

static void tcp_client_write(struct async_tcp_client_t *self_p,
                             const void *buf_p,
                             size_t size)
{
    ssize_t res;

    res = write(tcp_client(self_p)->sockfd, buf_p, size);

    if (res != (ssize_t)size) {
    }
}

static size_t tcp_client_read(struct async_tcp_client_t *self_p,
                              void *buf_p,
                              size_t size)
{
    ssize_t res;

    res = read(tcp_client(self_p)->sockfd, buf_p, size);

    if (res == 0) {
        async_tcp_client_data_complete_write(self_p, true);
    } else if (res == -1) {
        async_tcp_client_data_complete_write(self_p, false);
        res = 0;
    } else {
        async_call(self_p->async_p,
                   (async_func_t)tcp_client(self_p)->on_input,
                   self_p);
    }

    return (res);
}

struct async_runtime_t *async_runtime_linux_create()
{
    int sockets[2];
    int res;
    struct async_runtime_t *runtime_p;
    struct async_runtime_linux_t *self_p;

    runtime_p = malloc(sizeof(*runtime_p));

    if (runtime_p == NULL) {
        return (NULL);
    }

    runtime_p->set_async = (async_runtime_set_async_t)set_async;
    runtime_p->spawn = (async_runtime_spawn_t)spawn;
    runtime_p->run_forever = (async_runtime_run_forever_t)run_forever;
    runtime_p->tcp_client.init = tcp_client_init;
    runtime_p->tcp_client.connect = tcp_client_connect;
    runtime_p->tcp_client.disconnect = tcp_client_disconnect;
    runtime_p->tcp_client.write = tcp_client_write;
    runtime_p->tcp_client.read = tcp_client_read;
    self_p = malloc(sizeof(*self_p));

    if (self_p == NULL) {
        return (NULL);
    }

    res = socketpair(AF_UNIX, SOCK_STREAM, 0, &sockets[0]);

    if (res != 0) {
        return (NULL);
    }

    self_p->io_fd = sockets[0];
    self_p->async_fd = sockets[1];
    runtime_p->obj_p = self_p;

    return (runtime_p);
}
