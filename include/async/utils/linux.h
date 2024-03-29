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

#ifndef ASYNC_UTILS_LINUX_H
#define ASYNC_UTILS_LINUX_H

#include "async/core.h"

/**
 * Create a periodic timer and start it with the async period. Returns
 * the timer file descriptor.
 */
int async_utils_linux_create_periodic_timer(struct async_t *async_p);

/**
 * Called when the timer expirs. Reads from the timer file descriptor
 * and calls the async tick function.
 */
void async_utils_linux_handle_timeout(struct async_t *async_p,
                                      int timer_fd);

/**
 * Both creates and add the timer file descriptor to given epoll
 * instance. Returns the timer file descriptor.
 */
int async_utils_linux_init_periodic_timer(struct async_t *async_p,
                                          int epoll_fd);

/**
 * Create an epoll instance and return its handle.
 */
int async_utils_linux_epoll_create(void);

/**
 * Add given file descriptor to given epoll instance with EPOLLIN.
 */
void async_utils_linux_epoll_add_in(int epoll_fd, int fd);

void async_utils_linux_channel_stdin_init(struct async_channel_t *channel_p,
                                          struct async_t *async_p);

void async_utils_linux_channel_stdin_handle(struct async_channel_t *channel_p);

void async_utils_linux_fatal_perror(const char *message_p);

void async_utils_linux_init_stdin(int epoll_fd);

void async_utils_linux_make_stdin_unbuffered(void);

#endif
