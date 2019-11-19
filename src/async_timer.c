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

void async_timer_init(struct async_timer_t *self_p,
                      struct async_t *async_p,
                      int timeout_ms,
                      struct async_uid_t *message_p,
                      struct async_task_t *task_p,
                      int flags)
{
    (void)self_p;
    (void)async_p;
    (void)timeout_ms;
    (void)message_p;
    (void)task_p;
    (void)flags;
}

void async_timer_start(struct async_timer_t *self_p)
{
    (void)self_p;
}

void async_timer_stop(struct async_timer_t *self_p)
{
    (void)self_p;
}

bool async_timer_is_stopped(struct async_timer_t *self_p)
{
    (void)self_p;

    return (false);
}
