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
#include <sys/epoll.h>
#include "async.h"
#include "async/utils/linux.h"
#include "my_shell.h"

static void make_stdin_unbuffered(void)
{
    struct termios ctrl;

    tcgetattr(STDIN_FILENO, &ctrl);
    ctrl.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &ctrl);
}

int main()
{
    int epoll_fd;
    int timer_fd;
    int nfds;
    struct async_t async;
    struct my_shell_t my_shell;
    struct epoll_event event;
    struct async_channel_t channel;

    async_init(&async, 100);
    async_utils_linux_channel_stdin_init(&channel, &async);
    my_shell_init(&my_shell, &channel, &async);

    epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        exit(1);
    }

    timer_fd = async_utils_linux_init_periodic_timer(&async, epoll_fd);
    async_utils_linux_init_stdin(epoll_fd);
    make_stdin_unbuffered();

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
