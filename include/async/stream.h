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

#ifndef ASYNC_STREAM_H
#define ASYNC_STREAM_H

#include "async.h"

struct async_stream_t;

typedef void (*async_stream_open_t)(struct async_stream_t *self_p);
typedef void (*async_stream_close_t)(struct async_stream_t *self_p);
typedef ssize_t (*async_stream_read_t)(struct async_stream_t *self_p,
                                       void *buf_p,
                                       size_t size);
typedef ssize_t (*async_stream_write_t)(struct async_stream_t *self_p,
                                        const void *buf_p,
                                        size_t size);

struct async_stream_t {
    async_stream_open_t open;
    async_stream_close_t close;
    async_stream_read_t read;
    async_stream_write_t write;
    struct {
        async_func_t opened;
        async_func_t closed;
        async_func_t data;
        void *obj_p;
    } on;
    struct async_t *async_p;
};

/**
 * Initialize given stream.
 */
void async_stream_init(struct async_stream_t *self_p,
                       async_stream_open_t open_fn,
                       async_stream_close_t close_fn,
                       async_stream_read_t read_fn,
                       async_stream_write_t write_fn,
                       struct async_t *async_p);

void async_stream_set_on(struct async_stream_t *self_p,
                         async_func_t on_opened,
                         async_func_t on_closed,
                         async_func_t on_data,
                         void *obj_p);

void async_stream_open(struct async_stream_t *self_p);

void async_stream_close(struct async_stream_t *self_p);

size_t async_stream_read(struct async_stream_t *self_p,
                         void *buf_p,
                         size_t size);

ssize_t async_stream_write(struct async_stream_t *self_p,
                           const void *buf_p,
                           size_t size);

void async_stream_opened(struct async_stream_t *self_p);

void async_stream_closed(struct async_stream_t *self_p);

void async_stream_data(struct async_stream_t *self_p);

#endif
