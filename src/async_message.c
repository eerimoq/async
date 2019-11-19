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

void *async_message_alloc(struct async_t *async_p,
                          struct async_uid_t *uid_p,
                          size_t size)
{
    (void)async_p;

    struct async_message_header_t *header_p;

    header_p = malloc(sizeof(*header_p) + size);
    header_p->count = 1;
    header_p->uid_p = uid_p;
    header_p->on_free = NULL;

    return (message_from_header(header_p));
}

void async_message_set_on_free(void *message_p,
                               async_message_on_free_t on_free)
{
    struct async_message_header_t *header_p;

    header_p = message_to_header(message_p);
    header_p->on_free = on_free;
}

void async_message_free(void *message_p)
{
    struct async_message_header_t *header_p;
    int count;

    header_p = message_to_header(message_p);

    header_p->count--;
    count = header_p->count;

    if (count == 0) {
        if (header_p->on_free != NULL) {
            header_p->on_free(message_p);
        }

        free(header_p);
    }
}

void async_message_share(void *message_p, int count)
{
    message_to_header(message_p)->count += count;
}
