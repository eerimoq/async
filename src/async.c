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

#include "async.h"
#include "internal.h"

void async_init(struct async_t *self_p,
                int tick_in_ms,
                void *buf_p,
                size_t size)
{
    (void)buf_p;
    (void)size;

    self_p->tick_in_ms = tick_in_ms;
}

void async_process(struct async_t *self_p)
{
    (void)self_p;
}

void async_tick(struct async_t *self_p)
{
    (void)self_p;
}

void async_task_init(struct async_task_t *self_p,
                     struct async_t *async_p,
                     async_task_on_message_t on_message)
{
    async_queue_init(&self_p->messages, 32);
    self_p->on_message = on_message;
    self_p->async_p = async_p;
}

int async_send(struct async_task_t *receiver_p, void *message_p)
{
    return (async_queue_put(&receiver_p->messages, message_p));
}
