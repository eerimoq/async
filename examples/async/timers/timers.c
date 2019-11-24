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

static void timers_on_timeout_1(struct timers_t *self_p)
{
    if (!async_timer_is_stopped(&self_p->timer_1)) {
        printf("Timer 1 expired.\n");
    }
}

static void timers_on_timeout_2(void)
{
    printf("Timer 2 expired.\n");
}

static void timers_on_timeout_3(struct timers_t *self_p)
{
    printf("Timer 3 expired. Stopping timer 1.\n");
    async_timer_stop(&self_p->timer_1);
}

void timers_init(struct timers_t *self_p, struct async_t *async_p)
{
    async_timer_init(&self_p->timer_1,
                     (async_func_t)timers_on_timeout_1,
                     self_p,
                     ASYNC_TIMER_PERIODIC,
                     async_p);
    async_timer_init(&self_p->timer_2,
                     (async_func_t)timers_on_timeout_2,
                     self_p,
                     ASYNC_TIMER_PERIODIC,
                     async_p);
    async_timer_init(&self_p->timer_3,
                     (async_func_t)timers_on_timeout_3,
                     self_p,
                     0,
                     async_p);
    async_timer_start(&self_p->timer_1, 1000);
    async_timer_start(&self_p->timer_2, 3000);
    async_timer_start(&self_p->timer_3, 5000);
}
