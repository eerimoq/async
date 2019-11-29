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

/*
 * A channel is typically used to use file descriptors in the async
 * code.
 *
 * A use case:
 *
 * 1. Async code calls async_channel_open().
 *
 * 2. Main code opens a device.
 *
 * 3. Once opened, main code calls async_channel_opened(), which calls
 *    on_opened() in async code.
 *
 * 4. Once data is available, main code calls async_channel_input(),
 *    which calls on_input() in async code.
 *
 * 5. Async code calls async_channel_read() to read the data.
 */

#ifndef ASYNC_CHANNEL_H
#define ASYNC_CHANNEL_H

#include "async.h"

struct async_channel_t;

typedef void (*async_channel_open_t)(struct async_channel_t *self_p);
typedef void (*async_channel_close_t)(struct async_channel_t *self_p);
typedef ssize_t (*async_channel_read_t)(struct async_channel_t *self_p,
                                        void *buf_p,
                                        size_t size);
typedef ssize_t (*async_channel_write_t)(struct async_channel_t *self_p,
                                         const void *buf_p,
                                         size_t size);

struct async_channel_t {
    async_channel_open_t open;
    async_channel_close_t close;
    async_channel_read_t read;
    async_channel_write_t write;
    struct {
        async_func_t opened;
        async_func_t closed;
        async_func_t input;
        void *obj_p;
    } on;
    struct async_t *async_p;
};

/**
 * Initialize given channel.
 */
void async_channel_init(struct async_channel_t *self_p,
                        async_channel_open_t open_fn,
                        async_channel_close_t close_fn,
                        async_channel_read_t read_fn,
                        async_channel_write_t write_fn,
                        struct async_t *async_p);

/**
 * Set the on event callbacks. on_closed() is called when the peer
 * closes the channel. on_input() in salled the the peer writes data
 * to the channel.
 */
void async_channel_set_on(struct async_channel_t *self_p,
                          async_func_t on_opened,
                          async_func_t on_closed,
                          async_func_t on_input,
                          void *obj_p);

/**
 * Open the channel. on_opened() is called on completion.
 */
void async_channel_open(struct async_channel_t *self_p);

/**
 * Close the channel.
 */
void async_channel_close(struct async_channel_t *self_p);

/**
 * Read data from given channel.
 */
size_t async_channel_read(struct async_channel_t *self_p,
                          void *buf_p,
                          size_t size);

/**
 * Write data to given channel.
 */
ssize_t async_channel_write(struct async_channel_t *self_p,
                            const void *buf_p,
                            size_t size);

/**
 * Call when opened.
 */
void async_channel_opened(struct async_channel_t *self_p);

/**
 * Call when closed be the peer.
 */
void async_channel_closed(struct async_channel_t *self_p);

/**
 * Call when input is available.
 */
void async_channel_input(struct async_channel_t *self_p);

#endif
