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
#include <sys/eventfd.h>
#include <stdio.h>
#include "async.h"
#include "async/utils/linux.h"
#include "ml/ml.h"

static ML_UID(uid_timeout);
static ML_UID(uid_tcp_connect);
static ML_UID(uid_tcp_connect_complete);
static ML_UID(uid_tcp_disconnect);
static ML_UID(uid_tcp_data);
static ML_UID(uid_tcp_data_complete);
static ML_UID(uid_tcp_disconnected);
static ML_UID(uid_worker_job);
static ML_UID(uid_call_threadsafe);

struct call_threadsafe_t {
    async_func_t func;
    void *obj_p;
};

struct worker_job_t {
    async_func_t entry;
    void *obj_p;
    async_func_t on_complete;
    struct ml_queue_t *async_queue_p;
};

struct async_runtime_linux_t {
    struct {
        int fd;
        struct ml_queue_t queue;
        pthread_t pthread;
    } io;
    struct {
        struct ml_queue_t queue;
        pthread_t pthread;
    } async;
    struct ml_worker_pool_t worker_pool;
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

static void io_handle_timeout(struct async_runtime_linux_t *self_p,
                              int timer_fd)
{
    uint64_t value;
    void *message_p;

    read_buf(timer_fd, &value, sizeof(value));
    message_p = ml_message_alloc(&uid_timeout, 0);
    ml_queue_put(&self_p->async.queue, message_p);
}

static void io_handle_tcp_client_connect(struct async_runtime_linux_t *self_p,
                                         int epoll_fd,
                                         struct message_connect_t *req_p)
{
    struct sockaddr_in addr;
    int sockfd;
    struct message_connect_complete_t *rsp_p;
    int res;
    struct epoll_event event;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(req_p->port);
    inet_aton(req_p->host_p, (struct in_addr *)&addr.sin_addr.s_addr);
    free(req_p->host_p);

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

    rsp_p = ml_message_alloc(&uid_tcp_connect_complete, sizeof(*rsp_p));
    rsp_p->tcp_p = req_p->tcp_p;
    rsp_p->sockfd = sockfd;
    tcp_p = req_p->tcp_p;
    ml_queue_put(&self_p->async.queue, rsp_p);
}

static void io_handle_tcp_client_disconnect(struct async_runtime_linux_t *self_p,
                                            int epoll_fd)
{
    (void)self_p;

    close(tcp_client(tcp_p)->sockfd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, tcp_client(tcp_p)->sockfd, NULL);
}

static void io_handle_tcp_client_data_complete(struct async_runtime_linux_t *self_p,
                                               int epoll_fd,
                                               struct message_data_complete_t *ind_p)
{
    int sockfd;
    struct epoll_event event;
    void *message_p;

    sockfd = tcp_client(ind_p->tcp_p)->sockfd;

    if (ind_p->closed) {
        close(sockfd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, NULL);
        message_p = ml_message_alloc(&uid_tcp_disconnected, 0);
        ml_queue_put(&self_p->async.queue, message_p);
    } else {
        event.events = EPOLLIN;
        event.data.fd = sockfd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
    }
}

static void io_handle_async(struct async_runtime_linux_t *self_p,
                            int epoll_fd)
{
    struct ml_uid_t *uid_p;
    void *message_p;
    uint64_t value;

    read_buf(self_p->io.fd, &value, sizeof(value));
    uid_p = ml_queue_get(&self_p->io.queue, &message_p);

    if (uid_p == &uid_tcp_connect) {
        io_handle_tcp_client_connect(self_p, epoll_fd, message_p);
    } else if (uid_p == &uid_tcp_disconnect) {
        io_handle_tcp_client_disconnect(self_p, epoll_fd);
    } else if (uid_p == &uid_tcp_data_complete) {
        io_handle_tcp_client_data_complete(self_p, epoll_fd, message_p);
    }

    ml_message_free(message_p);
}

static void io_handle_socket(struct async_runtime_linux_t *self_p,
                             int epoll_fd)
{
    struct epoll_event event;
    void *message_p;

    event.events = 0;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, tcp_client(tcp_p)->sockfd, &event);
    message_p = ml_message_alloc(&uid_tcp_data, 0);
    ml_queue_put(&self_p->async.queue, message_p);
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
    event.data.fd = self_p->io.fd;
    res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, self_p->io.fd, &event);

    if (res == -1) {
        return (NULL);
    }

    while (true) {
        nfds = epoll_wait(epoll_fd, &event, 1, -1);

        if (nfds == 1) {
            if (event.data.fd == timer_fd) {
                io_handle_timeout(self_p, timer_fd);
            } else if (event.data.fd == self_p->io.fd) {
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

static void async_handle_tcp_client_connected(
    struct message_connect_complete_t *message_p)
{
    int res;

    async_tcp_client_set_sockfd(message_p->tcp_p, message_p->sockfd);
    res = (message_p->sockfd == -1 ? -1 : 0);
    tcp_client(message_p->tcp_p)->on_connected.func(message_p->tcp_p, res);
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

static void async_handle_worker_job(struct worker_job_t *job_p)
{
    job_p->on_complete(job_p->obj_p);
}

static void async_handle_call_threadsafe(struct call_threadsafe_t *message_p)
{
    message_p->func(message_p->obj_p);
}

static void *async_main(struct async_runtime_linux_t *self_p)
{
    struct ml_uid_t *uid_p;
    void *message_p;

    while (true) {
        uid_p = ml_queue_get(&self_p->async.queue, &message_p);

        if (uid_p == &uid_timeout) {
            async_handle_timeout(self_p);
        } else if (uid_p == &uid_tcp_connect_complete) {
            async_handle_tcp_client_connected(message_p);
        } else if (uid_p == &uid_tcp_data) {
            async_handle_tcp_client_input();
        } else if (uid_p == &uid_tcp_disconnected) {
            async_handle_tcp_client_disconnected();
        } else if (uid_p == &uid_worker_job) {
            async_handle_worker_job(message_p);
        } else if (uid_p == &uid_call_threadsafe) {
            async_handle_call_threadsafe(message_p);
        }

        ml_message_free(message_p);
        async_process(self_p->async_p);
    }

    return (NULL);
}

static void set_async(struct async_runtime_linux_t *self_p,
                      struct async_t *async_p)
{
    self_p->async_p = async_p;
}

static void call_threadsafe(struct async_runtime_linux_t *self_p,
                            async_func_t func,
                            void *obj_p)
{
    struct call_threadsafe_t *message_p;

    message_p = ml_message_alloc(&uid_call_threadsafe, sizeof(*message_p));
    message_p->func = func;
    message_p->obj_p = obj_p;
    ml_queue_put(&self_p->async.queue, message_p);

}

static void job(struct worker_job_t *job_p)
{
    job_p->entry(job_p->obj_p);
    ml_queue_put(job_p->async_queue_p, job_p);
}

static int call_worker_pool(struct async_runtime_linux_t *self_p,
                            async_func_t entry,
                            void *obj_p,
                            async_func_t on_complete)
{
    struct worker_job_t *job_p;

    job_p = ml_message_alloc(&uid_worker_job, sizeof(*job_p));
    job_p->entry = entry;
    job_p->obj_p = obj_p;
    job_p->on_complete = on_complete;
    job_p->async_queue_p = &self_p->async.queue;
    ml_worker_pool_spawn(&self_p->worker_pool,
                         (ml_worker_pool_job_entry_t)job,
                         job_p);

    return (0);
}

static void run_forever(struct async_runtime_linux_t *self_p)
{
    pthread_create(&self_p->io.pthread,
                   NULL,
                   (void *(*)(void *))io_main,
                   self_p);
    pthread_create(&self_p->async.pthread,
                   NULL,
                   (void *(*)(void *))async_main,
                   self_p);
    pthread_join(self_p->io.pthread, NULL);
    pthread_join(self_p->async.pthread, NULL);
}

void async_tcp_client_connect_write(struct async_tcp_client_t *self_p,
                                    const char *host_p,
                                    int port)
{
    struct message_connect_t *data_p;

    data_p = ml_message_alloc(&uid_tcp_connect, sizeof(*data_p));
    data_p->tcp_p = self_p;
    data_p->host_p = strdup(host_p);
    data_p->port = port;
    ml_queue_put(&tcp_runtime(self_p)->io.queue, data_p);
}

void async_tcp_client_disconnect_write(struct async_tcp_client_t *self_p)
{
    struct message_disconnect_t *data_p;

    data_p = ml_message_alloc(&uid_tcp_disconnect, sizeof(*data_p));
    data_p->sockfd = tcp_client(self_p)->sockfd;
    ml_queue_put(&tcp_runtime(self_p)->io.queue, data_p);
}

void async_tcp_client_data_complete_write(struct async_tcp_client_t *self_p,
                                          bool closed)
{
    struct message_data_complete_t *ind_p;

    ind_p = ml_message_alloc(&uid_tcp_data_complete, sizeof(*ind_p));
    ind_p->tcp_p = self_p;
    ind_p->closed = closed;
    ml_queue_put(&tcp_runtime(self_p)->io.queue, ind_p);
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
    (void)res;
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

static void on_put(int *fd_p)
{
    uint64_t value;
    ssize_t size;

    value = 1;
    size = write(*fd_p, &value, sizeof(value));
    (void)size;
}

struct async_runtime_t *async_runtime_linux_create()
{
    struct async_runtime_t *runtime_p;
    struct async_runtime_linux_t *self_p;

    runtime_p = malloc(sizeof(*runtime_p));

    if (runtime_p == NULL) {
        return (NULL);
    }

    runtime_p->set_async = (async_runtime_set_async_t)set_async;
    runtime_p->call_threadsafe = (async_runtime_call_threadsafe_t)call_threadsafe;
    runtime_p->call_worker_pool = (async_runtime_call_worker_pool_t)call_worker_pool;
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

    self_p->io.fd = eventfd(0, EFD_SEMAPHORE);

    if (self_p->io.fd == -1) {
        return (NULL);
    }

    ml_queue_init(&self_p->io.queue, 32);
    ml_queue_set_on_put(&self_p->io.queue, (ml_queue_put_t)on_put, &self_p->io.fd);
    ml_queue_init(&self_p->async.queue, 32);
    ml_worker_pool_init(&self_p->worker_pool, 4, 32);
    runtime_p->obj_p = self_p;

    return (runtime_p);
}
