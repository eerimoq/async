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

#include <stdio.h>
#include "timers.h"

static void on_timeout_1(struct async_timer_t *timer_p)
{
    struct timers_t *self_p;

    self_p = async_container_of(timer_p, typeof(*self_p), timer_1);
    printf("Timer 1 expired.\n");
}

static void on_timeout_2()
{
    printf("Timer 2 expired.\n");
}

static void on_timeout_3(struct async_timer_t *timer_p)
{
    struct timers_t *self_p;

    self_p = async_container_of(timer_p, typeof(*self_p), timer_3);
    printf("Timer 3 expired. Stopping timer 1.\n");
    async_timer_stop(&self_p->timer_1);
}

void timers_init(struct timers_t *self_p, struct async_t *async_p)
{
    async_timer_init(&self_p->timer_1,
                     on_timeout_1,
                     1000,
                     1000,
                     async_p);
    async_timer_init(&self_p->timer_2,
                     on_timeout_2,
                     3000,
                     3000,
                     async_p);
    async_timer_init(&self_p->timer_3,
                     on_timeout_3,
                     5000,
                     5000,
                     async_p);
    async_timer_start(&self_p->timer_1);
    async_timer_start(&self_p->timer_2);
    async_timer_start(&self_p->timer_3);
}
