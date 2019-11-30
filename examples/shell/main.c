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

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/epoll.h>
#include "async.h"
#include "async/utils/linux.h"

static int command_hello(struct async_shell_t *self_p,
                         int argc,
                         const char *argv[])
{
    (void)self_p;

    const char *name_p;

    if (argc == 2) {
        name_p = argv[1];
    } else {
        name_p = "stranger";
    }

    printf("Hello %s!\n", name_p);

    return (0);
}

static int command_exit(struct async_shell_t *self_p,
                        int argc,
                        const char *argv[])
{
    (void)self_p;
    (void)argc;
    (void)argv;

    exit(0);

    return (0);
}

static int init_periodic_timer(struct async_t *async_p,
                               int epoll_fd)
{
    int res;
    int timer_fd;
    struct epoll_event event;

    timer_fd = async_utils_linux_create_periodic_timer(async_p);

    if (timer_fd == -1) {
        exit(1);
    }

    event.events = EPOLLIN;
    event.data.fd = timer_fd;
    res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event);

    if (res == -1) {
        exit(1);
    }

    return (timer_fd);
}

static void make_stdin_unbuffered(void)
{
    struct termios ctrl;

    tcgetattr(STDIN_FILENO, &ctrl);
    ctrl.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &ctrl);
}

static void init_stdin(int epoll_fd)
{
    int res;
    struct epoll_event event;

    res = fcntl(fileno(stdin),
                F_SETFL,
                fcntl(fileno(stdin), F_GETFL) | O_NONBLOCK);

    if (res == -1) {
        exit(1);
    }

    event.events = EPOLLIN;
    event.data.fd = fileno(stdin);
    res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fileno(stdin), &event);

    if (res == -1) {
        exit(1);
    }

    make_stdin_unbuffered();
}

int main()
{
    struct async_t async;
    int timer_fd;
    struct async_shell_t shell;
    int epoll_fd;
    struct epoll_event event;
    int nfds;
    struct async_channel_t channel;

    async_init(&async, 100);
    async_utils_linux_channel_stdin_init(&channel, &async);
    async_shell_init(&shell, &channel, &async);
    async_shell_register_command(&shell,
                                 "hello",
                                 "My command.",
                                 command_hello);
    async_shell_register_command(&shell,
                                 "exit",
                                 "Exit the program.",
                                 command_exit);
    async_shell_start(&shell);

    epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        exit(1);
    }

    timer_fd = init_periodic_timer(&async, epoll_fd);
    init_stdin(epoll_fd);

    while (true) {
        nfds = epoll_wait(epoll_fd, &event, 1, -1);

        if (nfds == 1) {
            if (event.data.fd == timer_fd) {
                async_utils_linux_handle_timeout(&async, timer_fd);
            } else if (event.data.fd == fileno(stdin)) {
                async_utils_linux_channel_stdin_handle(&channel);
            }
        }

        async_process(&async);
    }

    return (1);
}
