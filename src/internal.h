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

#ifndef ASYNC_INTERNAL_H
#define ASYNC_INTERNAL_H

#include "async.h"

#define offsetof(type, member) ((size_t) &((type *)0)->member)

#define container_of(ptr, type, member)                         \
    ({                                                          \
        const typeof( ((type *)0)->member) *__mptr = (ptr);     \
        (type *)( (char *)__mptr - offsetof(type,member) );     \
    })

static inline struct async_message_header_t *message_to_header(void *message_p)
{
    return (&((struct async_message_header_t *)message_p)[-1]);
}

static inline void *message_from_header(struct async_message_header_t *header_p)
{
    return (&header_p[1]);
}

void async_message_free(void *message_p);

void async_queue_init(struct async_queue_t *self_p, int length);

struct async_uid_t *async_queue_get(struct async_queue_t *self_p,
                                    struct async_task_t **receiver_p,
                                    void **message_pp);

int async_queue_put(struct async_queue_t *self_p,
                    struct async_task_t *receiver_p,
                    void *message_p);

void async_timer_list_init(struct async_timer_list_t *self_p);

void async_timer_list_tick(struct async_timer_list_t *self_p);

#endif
