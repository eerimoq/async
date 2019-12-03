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
#include "my_shell.h"

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

void my_shell_init(struct my_shell_t *self_p,
                   struct async_channel_t *channel_p,
                   struct async_t *async_p)
{
    async_shell_init(&self_p->shell, channel_p, async_p);
    async_shell_command_init(&self_p->hello,
                             "hello",
                             "My command.",
                             command_hello);
    async_shell_register_command(self_p, &self_p->hello);
    async_shell_start(&self_p->shell);
}
