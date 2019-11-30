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

#include "async/channel.h"

static void on_opened(struct async_channel_t *self_p)
{
    self_p->on.opened.func(self_p->on.obj_p, self_p->on.opened.res);
}

static void null_open(struct async_channel_t *self_p)
{
    self_p->on.opened.res = 0;
    async_call(self_p->async_p, (async_func_t)on_opened, self_p);
}

static void null_close(struct async_channel_t *self_p)
{
    async_call(self_p->async_p, self_p->on.closed, self_p->on.obj_p);
}

static ssize_t null_read(struct async_channel_t *self_p,
                         void *buf_p,
                         size_t size)
{
    (void)self_p;
    (void)buf_p;

    return (size);
}

static ssize_t null_write(struct async_channel_t *self_p,
                          const void *buf_p,
                          size_t size)
{
    (void)self_p;
    (void)buf_p;

    return (size);
}

static void null_on()
{
}

void async_channel_init(struct async_channel_t *self_p,
                        async_channel_open_t open_func,
                        async_channel_close_t close_func,
                        async_channel_read_t read_func,
                        async_channel_write_t write_func,
                        struct async_t *async_p)
{
    if (open_func == NULL) {
        open_func = null_open;
    }

    if (close_func == NULL) {
        close_func = null_close;
    }

    if (read_func == NULL) {
        read_func = null_read;
    }

    if (write_func == NULL) {
        write_func = null_write;
    }

    self_p->open = open_func;
    self_p->close = close_func;
    self_p->read = read_func;
    self_p->write = write_func;
    self_p->on.opened.func = null_on;
    self_p->on.closed = null_on;
    self_p->on.input = null_on;
    self_p->on.obj_p = NULL;
    self_p->async_p = async_p;
}

void async_channel_set_on(struct async_channel_t *self_p,
                          async_channel_opened_t on_opened,
                          async_func_t on_closed,
                          async_func_t on_input,
                          void *obj_p)
{
    if (on_opened == NULL) {
        on_opened = null_on;
    }

    if (on_closed == NULL) {
        on_closed = null_on;
    }

    if (on_input == NULL) {
        on_input = null_on;
    }

    self_p->on.opened.func = on_opened;
    self_p->on.closed = on_closed;
    self_p->on.input = on_input;
    self_p->on.obj_p = obj_p;
}

void async_channel_open(struct async_channel_t *self_p)
{
    return (self_p->open(self_p));
}

void async_channel_close(struct async_channel_t *self_p)
{
    return (self_p->close(self_p));
}

size_t async_channel_read(struct async_channel_t *self_p,
                          void *buf_p,
                          size_t size)
{
    return (self_p->read(self_p, buf_p, size));
}

ssize_t async_channel_write(struct async_channel_t *self_p,
                            const void *buf_p,
                            size_t size)
{
    return (self_p->write(self_p, buf_p, size));
}

void async_channel_opened(struct async_channel_t *self_p, int res)
{
    self_p->on.opened.res = res;
    async_call(self_p->async_p, (async_func_t)on_opened, self_p);
}

void async_channel_closed(struct async_channel_t *self_p)
{
    async_call(self_p->async_p, self_p->on.closed, self_p->on.obj_p);
}

void async_channel_input(struct async_channel_t *self_p)
{
    async_call(self_p->async_p, self_p->on.input, self_p->on.obj_p);
}
