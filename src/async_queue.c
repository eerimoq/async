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

static bool is_empty(struct async_queue_t *self_p)
{
    return (self_p->rdpos == self_p->wrpos);
}

static bool is_full(struct async_queue_t *self_p)
{
    return (((self_p->wrpos + 1) % self_p->length) == self_p->rdpos);
}

static void push_message(struct async_queue_t *self_p,
                         struct async_task_t *receiver_p,
                         struct async_message_header_t *header_p)
{
    self_p->messages_p[self_p->wrpos].receiver_p = receiver_p;
    self_p->messages_p[self_p->wrpos].message_p = header_p;
    self_p->wrpos++;
    self_p->wrpos %= self_p->length;
}

static struct async_task_t *pop_message(struct async_queue_t *self_p,
                                        struct async_message_header_t **header_pp)
{
    struct async_task_t *receiver_p;

    receiver_p = self_p->messages_p[self_p->rdpos].receiver_p;
    *header_pp = self_p->messages_p[self_p->rdpos].message_p;
    self_p->rdpos++;
    self_p->rdpos %= self_p->length;

    return (receiver_p);
}

void async_queue_init(struct async_queue_t *self_p, int length)
{
    self_p->rdpos = 0;
    self_p->wrpos = 0;
    self_p->length = (length + 1);
    self_p->messages_p = malloc(sizeof(*self_p->messages_p) * self_p->length);
}

struct async_uid_t *async_queue_get(struct async_queue_t *self_p,
                                    struct async_task_t **receiver_pp,
                                    void **message_pp)
{
    struct async_message_header_t *header_p;

    if (is_empty(self_p)) {
        return (NULL);
    }

    *receiver_pp = pop_message(self_p, &header_p);
    *message_pp = message_from_header(header_p);

    return (header_p->uid_p);
}

int async_queue_put(struct async_queue_t *self_p,
                    struct async_task_t *receiver_p,
                    void *message_p)
{
    if (is_full(self_p)) {
        return (ASYNC_ERROR_QUEUE_FULL);
    }

    push_message(self_p, receiver_p, message_to_header(message_p));

    return (0);
}
