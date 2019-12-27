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
#include <stdio.h>
#include <sys/epoll.h>
#include "async.h"
#include "async/utils/linux.h"
#include "counter.h"

static void handle_stdin(struct counter_t *counter_p)
{
    char ch;
    ssize_t res;

    res = read(fileno(stdin), &ch, sizeof(ch));

    if (res != sizeof(ch)) {
        exit(1);
    }

    switch (ch) {

    case '+':
        counter_incremect(counter_p);
        break;

    case '-':
        counter_decrement(counter_p);
        break;

    default:
        printf("Press + or -, not %c.\n", ch);
        break;
    }
}

int main()
{
    int timer_fd;
    int epoll_fd;
    int nfds;
    struct async_t async;
    struct counter_t counter;
    struct epoll_event event;

    async_init(&async);
    counter_init(&counter, &async);
    epoll_fd = async_utils_linux_epoll_create();
    timer_fd = async_utils_linux_init_periodic_timer(&async, epoll_fd);
    async_utils_linux_epoll_add_in(epoll_fd, fileno(stdin));
    async_utils_linux_make_stdin_unbuffered();

    printf("Press + and - to increment and decrement the counter.\n");

    while (true) {
        nfds = epoll_wait(epoll_fd, &event, 1, -1);

        if (nfds == 1) {
            if (event.data.fd == timer_fd) {
                async_utils_linux_handle_timeout(&async, timer_fd);
            } else if (event.data.fd == fileno(stdin)) {
                handle_stdin(&counter);
            }

            async_process(&async, 0);
        }
    }

    return (1);
}
