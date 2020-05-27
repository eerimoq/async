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
#include <sys/types.h>
#include "async.h"
#include "async/utils/linux.h"
#include "ml/ml.h"

static ML_UID(uid_timeout);
static ML_UID(uid_tcp_client_connect);
static ML_UID(uid_tcp_client_connect_complete);
static ML_UID(uid_tcp_client_disconnect);
static ML_UID(uid_tcp_client_data);
static ML_UID(uid_tcp_client_data_complete);
static ML_UID(uid_tcp_client_disconnected);
static ML_UID(uid_worker_job);
static ML_UID(uid_call_threadsafe);

struct call_threadsafe_t {
    async_func_t func;
    void *obj_p;
    void *arg_p;
};

struct worker_job_t {
    async_func_t entry;
    void *obj_p;
    void *arg_p;
    async_func_t on_complete;
    struct ml_queue_t *async_queue_p;
};

struct async_runtime_linux_t {
    struct async_runtime_t runtime;
    struct {
        int fd;
        int epoll_fd;
        struct ml_queue_t queue;
        pthread_t pthread;
    } io;
    struct {
        struct ml_timer_t timer;
        struct ml_queue_t queue;
        pthread_t pthread;
    } async;
    struct ml_timer_handler_t timer_handler;
    struct ml_worker_pool_t worker_pool;
    struct async_t *async_p;
};

typedef void (*io_epoll_func_t)(struct async_runtime_linux_t *self_p,
                                int epoll_fd,
                                void *arg_p);

struct io_epoll_data_t {
    io_epoll_func_t func;
    void *arg_p;
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
    int sockfd;
};

struct message_disconnected_t {
    struct async_tcp_client_t *tcp_p;
};

struct message_data_t {
    struct async_tcp_client_t *tcp_p;
};

struct message_data_complete_t {
    struct async_tcp_client_t *tcp_p;
};

struct tcp_client_t {
    async_tcp_client_connected_t on_connected;
    async_tcp_client_disconnected_t on_disconnected;
    async_tcp_client_input_t on_input;
    int sockfd;
    bool closed;
    struct io_epoll_data_t *epoll_data_p;
};

struct tcp_server_t {
    int listener;
    const char *host_p;
    int port;
    async_tcp_server_client_connected_t on_connected;
    async_tcp_server_client_disconnected_t on_disconnected;
    async_tcp_server_client_input_t on_input;
    struct io_epoll_data_t *epoll_data_p;
};

static struct io_epoll_data_t *io_epoll_data_create(io_epoll_func_t func,
                                                    void *arg_p)
{
    struct io_epoll_data_t *data_p;

    data_p = malloc(sizeof(*data_p));

    if (data_p == NULL) {
        async_utils_linux_fatal_perror("malloc");
    }

    data_p->func = func;
    data_p->arg_p = arg_p;

    return (data_p);
}

static struct tcp_client_t *tcp_client(struct async_tcp_client_t *self_p)
{
    return ((struct tcp_client_t *)(self_p->obj_p));
}

static struct async_runtime_linux_t *tcp_client_runtime(
    struct async_tcp_client_t *self_p)
{
    return ((struct async_runtime_linux_t *)(self_p->async_p->runtime_p->obj_p));
}

static void async_tcp_client_set_sockfd(struct async_tcp_client_t *self_p,
                                        int sockfd)
{
    tcp_client(self_p)->sockfd = sockfd;
}

static void io_handle_tcp_client(struct async_runtime_linux_t *self_p,
                                 int epoll_fd,
                                 struct async_tcp_client_t *tcp_p)
{
    struct epoll_event event;
    struct message_data_t *message_p;

    event.events = 0;
    event.data.ptr = tcp_client(tcp_p)->epoll_data_p;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, tcp_client(tcp_p)->sockfd, &event);
    message_p = ml_message_alloc(&uid_tcp_client_data, sizeof(*message_p));
    message_p->tcp_p = tcp_p;
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
        res = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

        if (res != -1) {
            res = fcntl(sockfd,
                        F_SETFL,
                        fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

            if (res != -1) {
                tcp_client(req_p->tcp_p)->epoll_data_p = io_epoll_data_create(
                    (io_epoll_func_t)io_handle_tcp_client,
                    req_p->tcp_p);
                event.events = EPOLLIN;
                event.data.ptr = tcp_client(req_p->tcp_p)->epoll_data_p;
                res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event);
            }
        }

        if (res == -1) {
            close(sockfd);
            sockfd = -1;
        }
    }

    rsp_p = ml_message_alloc(&uid_tcp_client_connect_complete, sizeof(*rsp_p));
    rsp_p->tcp_p = req_p->tcp_p;
    rsp_p->sockfd = sockfd;
    ml_queue_put(&self_p->async.queue, rsp_p);
}

static void io_handle_tcp_client_disconnect(int epoll_fd,
                                            struct message_disconnect_t *ind_p)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ind_p->sockfd, NULL);
    close(ind_p->sockfd);
}

static void io_handle_tcp_client_data_complete(struct async_runtime_linux_t *self_p,
                                               int epoll_fd,
                                               struct message_data_complete_t *ind_p)
{
    int sockfd;
    struct epoll_event event;
    struct message_disconnected_t *message_p;

    sockfd = tcp_client(ind_p->tcp_p)->sockfd;

    if (tcp_client(ind_p->tcp_p)->closed) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, NULL);
        close(sockfd);
        message_p = ml_message_alloc(&uid_tcp_client_disconnected,
                                     sizeof(*message_p));
        message_p->tcp_p = ind_p->tcp_p;
        ml_queue_put(&self_p->async.queue, message_p);
    } else {
        event.events = EPOLLIN;
        event.data.ptr = tcp_client(ind_p->tcp_p)->epoll_data_p;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
    }
}

static struct tcp_server_t *tcp_server(struct async_tcp_server_t *self_p)
{
    return ((struct tcp_server_t *)(self_p->obj_p));
}

static void io_handle_tcp_server_listener(struct async_runtime_linux_t *self_p,
                                          int epoll_fd,
                                          struct async_tcp_server_t *tcp_p)
{
    (void)self_p;
    (void)epoll_fd;

    int client_fd;

    client_fd = accept(tcp_server(tcp_p)->listener, NULL, 0);

    (void)client_fd;

    exit(5);
}

static void io_handle_async(struct async_runtime_linux_t *self_p,
                            int epoll_fd,
                            void *arg_p)
{
    (void)arg_p;

    struct ml_uid_t *uid_p;
    void *message_p;
    uint64_t event;
    ssize_t res;

    res = read(self_p->io.fd, &event, sizeof(event));

    if (res != (ssize_t)sizeof(event)) {
        async_utils_linux_fatal_perror("event read");
    }

    uid_p = ml_queue_get(&self_p->io.queue, &message_p);

    if (uid_p == &uid_tcp_client_connect) {
        io_handle_tcp_client_connect(self_p, epoll_fd, message_p);
    } else if (uid_p == &uid_tcp_client_disconnect) {
        io_handle_tcp_client_disconnect(epoll_fd, message_p);
    } else if (uid_p == &uid_tcp_client_data_complete) {
        io_handle_tcp_client_data_complete(self_p, epoll_fd, message_p);
    }

    ml_message_free(message_p);
}

static void *io_main(struct async_runtime_linux_t *self_p)
{
    ssize_t res;
    int nfds;
    struct epoll_event event;
    struct io_epoll_data_t *data_p;

    pthread_setname_np(pthread_self(), "async_io");

    event.events = EPOLLIN;
    event.data.ptr = io_epoll_data_create((io_epoll_func_t)io_handle_async, NULL);
    res = epoll_ctl(self_p->io.epoll_fd, EPOLL_CTL_ADD, self_p->io.fd, &event);

    if (res == -1) {
        return (NULL);
    }

    while (true) {
        nfds = epoll_wait(self_p->io.epoll_fd, &event, 1, -1);

        if (nfds == 1) {
            data_p = (struct io_epoll_data_t *)event.data.ptr;
            data_p->func(self_p, self_p->io.epoll_fd, data_p->arg_p);
        }
    }

    return (NULL);
}

static void async_handle_tcp_client_connected(
    struct message_connect_complete_t *message_p)
{
    int res;

    async_tcp_client_set_sockfd(message_p->tcp_p, message_p->sockfd);
    res = (message_p->sockfd == -1 ? -1 : 0);
    tcp_client(message_p->tcp_p)->on_connected(message_p->tcp_p, res);
}

static void async_tcp_client_data_complete_write(struct async_tcp_client_t *self_p)
{
    struct message_data_complete_t *ind_p;

    ind_p = ml_message_alloc(&uid_tcp_client_data_complete, sizeof(*ind_p));
    ind_p->tcp_p = self_p;
    ml_queue_put(&tcp_client_runtime(self_p)->io.queue, ind_p);
}

static void async_handle_tcp_client_data(struct message_data_t *req_p)
{
    tcp_client(req_p->tcp_p)->on_input(req_p->tcp_p);
    async_tcp_client_data_complete_write(req_p->tcp_p);
}

static void async_handle_timeout(struct async_runtime_linux_t *self_p)
{
    async_tick(self_p->async_p);
}

static void async_handle_tcp_client_disconnected(
    struct message_disconnected_t *ind_p)
{
    async_tcp_client_set_sockfd(ind_p->tcp_p, -1);
    tcp_client(ind_p->tcp_p)->on_disconnected(ind_p->tcp_p);
}

static void async_handle_worker_job(struct worker_job_t *job_p)
{
    job_p->on_complete(job_p->obj_p, job_p->arg_p);
}

static void async_handle_call_threadsafe(struct call_threadsafe_t *message_p)
{
    message_p->func(message_p->obj_p, message_p->arg_p);
}

static void *async_main(struct async_runtime_linux_t *self_p)
{
    struct ml_uid_t *uid_p;
    void *message_p;

    pthread_setname_np(pthread_self(), "async_async");

    ml_timer_handler_timer_start(&self_p->async.timer,
                                 0,
                                 self_p->async_p->tick_in_ms);

    while (true) {
        uid_p = ml_queue_get(&self_p->async.queue, &message_p);

        if (uid_p == &uid_timeout) {
            async_handle_timeout(self_p);
        } else if (uid_p == &uid_tcp_client_connect_complete) {
            async_handle_tcp_client_connected(message_p);
        } else if (uid_p == &uid_tcp_client_data) {
            async_handle_tcp_client_data(message_p);
        } else if (uid_p == &uid_tcp_client_disconnected) {
            async_handle_tcp_client_disconnected(message_p);
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
                            void *obj_p,
                            void *arg_p)
{
    struct call_threadsafe_t *message_p;

    message_p = ml_message_alloc(&uid_call_threadsafe, sizeof(*message_p));
    message_p->func = func;
    message_p->obj_p = obj_p;
    message_p->arg_p = arg_p;
    ml_queue_put(&self_p->async.queue, message_p);

}

static void job(struct worker_job_t *job_p)
{
    job_p->entry(job_p->obj_p, job_p->arg_p);
    ml_queue_put(job_p->async_queue_p, job_p);
}

static int call_worker_pool(struct async_runtime_linux_t *self_p,
                            async_func_t entry,
                            void *obj_p,
                            void *arg_p,
                            async_func_t on_complete)
{
    struct worker_job_t *job_p;

    job_p = ml_message_alloc(&uid_worker_job, sizeof(*job_p));
    job_p->entry = entry;
    job_p->obj_p = obj_p;
    job_p->arg_p = arg_p;
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

static void async_tcp_client_connect_write(struct async_tcp_client_t *self_p,
                                           const char *host_p,
                                           int port)
{
    struct message_connect_t *data_p;

    data_p = ml_message_alloc(&uid_tcp_client_connect, sizeof(*data_p));
    data_p->tcp_p = self_p;
    data_p->host_p = strdup(host_p);
    data_p->port = port;
    ml_queue_put(&tcp_client_runtime(self_p)->io.queue, data_p);
}

static void async_tcp_client_disconnect_write(struct async_tcp_client_t *self_p)
{
    struct message_disconnect_t *data_p;

    data_p = ml_message_alloc(&uid_tcp_client_disconnect, sizeof(*data_p));
    data_p->sockfd = tcp_client(self_p)->sockfd;
    ml_queue_put(&tcp_client_runtime(self_p)->io.queue, data_p);
}

static void tcp_client_init(struct async_tcp_client_t *self_p,
                            async_tcp_client_connected_t on_connected,
                            async_tcp_client_disconnected_t on_disconnected,
                            async_tcp_client_input_t on_input)
{
    struct tcp_client_t *rself_p;

    rself_p = malloc(sizeof(*rself_p));

    if (rself_p == NULL) {
        async_utils_linux_fatal_perror("tcp client malloc");
    }

    rself_p->on_connected = on_connected;
    rself_p->on_disconnected = on_disconnected;
    rself_p->on_input = on_input;
    rself_p->sockfd = -1;
    rself_p->closed = false;
    self_p->obj_p = rself_p;
}

static void tcp_client_connect(struct async_tcp_client_t *self_p,
                               const char *host_p,
                               int port)
{
    tcp_client(self_p)->sockfd = -1;
    tcp_client(self_p)->closed = false;
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
        fprintf(stderr, "TCP write failed %d %d.\n", (int)res, (int)size);
    }
}

static size_t tcp_client_read(struct async_tcp_client_t *self_p,
                              void *buf_p,
                              size_t size)
{
    ssize_t res;

    if (tcp_client(self_p)->closed) {
        return (0);
    }

    res = read(tcp_client(self_p)->sockfd, buf_p, size);

    if (res == 0) {
        tcp_client(self_p)->closed = true;
    } else if (res == -1) {
        res = 0;
    }

    return (res);
}

static struct async_runtime_linux_t *tcp_server_runtime(
    struct async_tcp_server_t *self_p)
{
    return ((struct async_runtime_linux_t *)(self_p->async_p->runtime_p->obj_p));
}

static void tcp_server_init(struct async_tcp_server_t *self_p,
                            const char *host_p,
                            int port,
                            async_tcp_server_client_connected_t on_connected,
                            async_tcp_server_client_disconnected_t on_disconnected,
                            async_tcp_server_client_input_t on_input)
{
    struct tcp_server_t *rself_p;

    rself_p = malloc(sizeof(*rself_p));

    if (rself_p == NULL) {
        async_utils_linux_fatal_perror("tcp server malloc");
    }

    rself_p->listener = -1;
    rself_p->host_p = strdup(host_p);
    rself_p->port = port;
    rself_p->on_connected = on_connected;
    rself_p->on_disconnected = on_disconnected;
    rself_p->on_input = on_input;
    self_p->obj_p = rself_p;
}

static void tcp_server_add_client(struct async_tcp_server_t *self_p,
                                  struct async_tcp_server_client_t *client_p)
{
    (void)self_p;
    (void)client_p;
}

static int tcp_server_start(struct async_tcp_server_t *self_p)
{
    struct sockaddr_in addr;
    int sockfd;
    int res;
    struct epoll_event event;
    struct tcp_server_t *rself_p;

    res = -1;
    rself_p = tcp_server(self_p);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(rself_p->port);
    inet_aton(rself_p->host_p, (struct in_addr *)&addr.sin_addr.s_addr);

    sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    if (sockfd != -1) {
        res = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

        if (res != -1) {
            res = listen(sockfd, 5);

            if (res != -1) {
                rself_p->epoll_data_p = io_epoll_data_create(
                    (io_epoll_func_t)io_handle_tcp_server_listener,
                    self_p);
                event.events = EPOLLIN;
                event.data.ptr = rself_p->epoll_data_p;
                res = epoll_ctl(tcp_server_runtime(self_p)->io.epoll_fd,
                                EPOLL_CTL_ADD,
                                sockfd,
                                &event);
            }
        }

        if (res == -1) {
            close(sockfd);
            sockfd = -1;
        }
    }

    rself_p->listener = sockfd;

    return (res);
}

static void tcp_server_stop(struct async_tcp_server_t *self_p)
{
    (void)self_p;

    exit(3);
}

static void tcp_server_client_write(struct async_tcp_server_client_t *self_p,
                                    const void *buf_p,
                                    size_t size)
{
    (void)self_p;
    (void)buf_p;
    (void)size;

    exit(4);
}

static size_t tcp_server_client_read(struct async_tcp_server_client_t *self_p,
                                     void *buf_p,
                                     size_t size)
{
    (void)self_p;
    (void)buf_p;
    (void)size;

    exit(5);

    return (0);
}

static void tcp_server_client_disconnect(struct async_tcp_server_client_t *self_p)
{
    (void)self_p;

    exit(6);
}

static void on_put_signal_event(int *fd_p)
{
    uint64_t value;
    ssize_t size;

    value = 1;
    size = write(*fd_p, &value, sizeof(value));
    (void)size;
}

static int init(struct async_runtime_linux_t *self_p)
{
    struct async_runtime_t *runtime_p;

    runtime_p = &self_p->runtime;
    runtime_p->set_async = (async_runtime_set_async_t)set_async;
    runtime_p->call_threadsafe = (async_runtime_call_threadsafe_t)call_threadsafe;
    runtime_p->call_worker_pool = (async_runtime_call_worker_pool_t)call_worker_pool;
    runtime_p->run_forever = (async_runtime_run_forever_t)run_forever;
    runtime_p->tcp_client.init = tcp_client_init;
    runtime_p->tcp_client.connect = tcp_client_connect;
    runtime_p->tcp_client.disconnect = tcp_client_disconnect;
    runtime_p->tcp_client.write = tcp_client_write;
    runtime_p->tcp_client.read = tcp_client_read;
    runtime_p->tcp_server.init = tcp_server_init;
    runtime_p->tcp_server.add_client = tcp_server_add_client;
    runtime_p->tcp_server.start = tcp_server_start;
    runtime_p->tcp_server.stop = tcp_server_stop;
    runtime_p->tcp_server.client.write = tcp_server_client_write;
    runtime_p->tcp_server.client.read = tcp_server_client_read;
    runtime_p->tcp_server.client.disconnect = tcp_server_client_disconnect;

    self_p->io.fd = eventfd(0, EFD_SEMAPHORE);

    if (self_p->io.fd == -1) {
        return (-1);
    }

    self_p->io.epoll_fd = epoll_create1(0);

    if (self_p->io.epoll_fd == -1) {
        return (-1);
    }

    ml_timer_handler_init(&self_p->timer_handler);
    ml_queue_init(&self_p->io.queue, 32);
    ml_queue_set_on_put(&self_p->io.queue,
                        (ml_queue_put_t)on_put_signal_event,
                        &self_p->io.fd);
    ml_timer_handler_timer_init(&self_p->timer_handler,
                                &self_p->async.timer,
                                &uid_timeout,
                                &self_p->async.queue);
    ml_queue_init(&self_p->async.queue, 32);
    ml_worker_pool_init(&self_p->worker_pool, 4, 32);
    runtime_p->obj_p = self_p;

    return (0);
}

struct async_runtime_t *async_runtime_linux_create()
{
    struct async_runtime_linux_t *self_p;
    int res;

    self_p = malloc(sizeof(*self_p));

    if (self_p == NULL) {
        return (NULL);
    }

    res = init(self_p);

    if (res != 0) {
        free(self_p);

        return (NULL);
    }

    return (&self_p->runtime);
}
