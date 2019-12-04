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
                      async_func_t on_timeout,
                      void *obj_p,
                      int flags,
                      struct async_t *async_p)
{
    self_p->async_p = async_p;
    self_p->on_timeout = on_timeout;
    self_p->obj_p = obj_p;
    self_p->flags = flags;
    self_p->stopped = false;
}

void async_timer_start(struct async_timer_t *self_p,
                       int timeout_ms)
{
    async_timer_stop(self_p);

    self_p->timeout = (timeout_ms / self_p->async_p->tick_in_ms);

    if (self_p->timeout == 0) {
        self_p->timeout = 1;
    }

    self_p->delta = self_p->timeout;
    self_p->stopped = false;

    /* Must wait at least two ticks to ensure the timer does not
       expire early since it may be started close to the next tick
       occurs. */
    self_p->delta++;

    timer_list_insert(&self_p->async_p->running_timers, self_p);
}

void async_timer_stop(struct async_timer_t *self_p)
{
    self_p->stopped = true;
    timer_list_remove(&self_p->async_p->running_timers, self_p);
}

bool async_timer_is_stopped(struct async_timer_t *self_p)
{
    return (self_p->stopped);
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
        async_call(timer_p->async_p,
                   timer_p->on_timeout,
                   timer_p->obj_p);

        /* Re-set periodic timers. */
        if (timer_p->flags & ASYNC_TIMER_PERIODIC) {
            timer_p->delta = timer_p->timeout;
            timer_list_insert(self_p, timer_p);
        }
    }
}
