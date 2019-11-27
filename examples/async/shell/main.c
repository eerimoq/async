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
#include "async.h"
#include "async/linux.h"
#include "timers.h"

/**
 * A shell command printing "Hello <name>!".
 */
static int command_hello(int argc, const char *argv[])
{
    const char *name_p;

    if (argc == 2) {
        name_p = argv[1];
    } else {
        name_p = "stranger";
    }

    printf("Hello %s!\n", name_p);

    return (0);
}

static void handle_stdin(struct async_t *async_p,
                         struct async_shell_t *shell_p)
{
    async_call(async_p, (async_func_t)shell_on_data, shell_p);
}

int main()
{
    struct async_t async;
    struct timers_t timers;
    int timer_fd;
    ssize_t res;
    uint64_t value;
    struct async_shell_t shell;
    int epoll_fd;
    struct epoll_event event;
    int nfds;

    async_init(&async, 100);
    async_linux_stream_stdin_init(&stream, &async);
    async_shell_init(&shell, &stream, &async);
    async_shell_register_command(&shell,
                                 "hello",
                                 "My command.",
                                 command_hello);
    async_shell_start(&shell);

    timer_fd = init_periodic_timer(&async, epoll_fd);

    while (true) {
        nfds = epoll_wait(epoll_fd, &event, 1, -1);

        if (nfds == 1) {
            if (event.data.fd == timer_fd) {
                async_linux_handle_timeout(&async, timer_fd);
            } else if (event.data.fd == fileno(stdin)) {
                async_linux_stream_stdin_handle(&stream);
            }
        }

        async_process(&async);
    }

    return (1);
}
