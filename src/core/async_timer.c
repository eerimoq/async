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
#include "async.h"
#include "internal.h"

static void on_timeout(struct async_timer_t *self_p)
{
    if (self_p->number_of_timeouts_to_ignore > 0) {
        self_p->number_of_timeouts_to_ignore--;

        return;
    }

    self_p->number_of_outstanding_timeouts--;
    self_p->on_timeout(self_p);
}

static void timer_list_insert(struct async_timer_list_t *self_p,
                              struct async_timer_t *timer_p)
{
    struct async_timer_t *elem_p;
    struct async_timer_t *prev_p;

    /* Find element preceeding this timer. */
    elem_p = self_p->head_p;
    prev_p = NULL;

    /* Find the element to insert this timer before. Delta is
       initially the timeout. */
    while (elem_p->delta < timer_p->delta) {
        timer_p->delta -= elem_p->delta;
        prev_p = elem_p;
        elem_p = elem_p->next_p;
    }

    /* Adjust the next timer for this timers delta. Do not adjust the
       tail timer. */
    if (elem_p != &self_p->tail) {
        elem_p->delta -= timer_p->delta;
    }

    /* Insert the new timer into list. */
    timer_p->next_p = elem_p;

    if (prev_p == NULL) {
        self_p->head_p = timer_p;
    } else {
        prev_p->next_p = timer_p;
    }
}

/**
 * Remove given timer from given list of active timers.
 */
static void timer_list_remove(struct async_timer_list_t *self_p,
                              struct async_timer_t *timer_p)
{
    struct async_timer_t *elem_p;
    struct async_timer_t *prev_p;

    /* Find element preceeding this timer.*/
    elem_p = self_p->head_p;
    prev_p = NULL;

    while (elem_p != NULL) {
        if (elem_p == timer_p) {
            /* Remove the timer from the list. */
            if (prev_p != NULL) {
                prev_p->next_p = elem_p->next_p;
            } else {
                self_p->head_p = elem_p->next_p;
            }

            /* Add the delta timeout to the next timer. */
            if (elem_p->next_p != &self_p->tail) {
                elem_p->next_p->delta += elem_p->delta;
            }

            return;
        }

        prev_p = elem_p;
        elem_p = elem_p->next_p;
    }
}

void async_timer_init(struct async_timer_t *self_p,
                      async_timer_timeout_t on_timeout,
                      unsigned int initial,
                      unsigned int repeat,
                      struct async_t *async_p)
{
    self_p->async_p = async_p;
    self_p->on_timeout = on_timeout;
    async_timer_set_initial(self_p, initial);
    async_timer_set_repeat(self_p, repeat);
    self_p->number_of_outstanding_timeouts = 0;
    self_p->number_of_timeouts_to_ignore = 0;
}

void async_timer_set_initial(struct async_timer_t *self_p,
                             unsigned int initial)
{
    self_p->initial = initial;
    self_p->initial_ticks = DIV_CEIL(initial, self_p->async_p->tick_in_ms);
    self_p->initial_ticks++;
}

unsigned int async_timer_get_initial(struct async_timer_t *self_p)
{
    return (self_p->initial);
}

void async_timer_set_repeat(struct async_timer_t *self_p,
                            unsigned int repeat)
{
    self_p->repeat = repeat;
    self_p->repeat_ticks = DIV_CEIL(repeat, self_p->async_p->tick_in_ms);
}

unsigned int async_timer_get_repeat(struct async_timer_t *self_p)
{
    return (self_p->repeat);
}

void async_timer_start(struct async_timer_t *self_p)
{
    async_timer_stop(self_p);
    self_p->delta = self_p->initial_ticks;
    timer_list_insert(&self_p->async_p->running_timers, self_p);
}

void async_timer_stop(struct async_timer_t *self_p)
{
    self_p->number_of_timeouts_to_ignore = self_p->number_of_outstanding_timeouts;
    timer_list_remove(&self_p->async_p->running_timers, self_p);
}

void async_timer_list_init(struct async_timer_list_t *self_p)
{
    self_p->head_p = &self_p->tail;
    self_p->tail.next_p = NULL;
    self_p->tail.delta = -1;
}

void async_timer_list_tick(struct async_timer_list_t *self_p)
{
    struct async_timer_t *timer_p;

    /* Return if no timers are active.*/
    if (self_p->head_p == &self_p->tail) {
        return;
    }

    /* Fire all expired timers.*/
    self_p->head_p->delta--;

    while (self_p->head_p->delta == 0) {
        timer_p = self_p->head_p;
        self_p->head_p = timer_p->next_p;
        timer_p->number_of_outstanding_timeouts++;
        async_call(timer_p->async_p, (async_func_t)on_timeout, timer_p);

        /* Re-set periodic timers. */
        if (timer_p->repeat_ticks > 0) {
            timer_p->delta = timer_p->repeat_ticks;
            timer_list_insert(self_p, timer_p);
        }
    }
}
