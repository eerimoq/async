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

#ifndef ASYNC_CORE_H
#define ASYNC_CORE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define ASYNC_TIMER_PERIODIC                     (1 << 0)

/* Error codes. */
#define ASYNC_ERROR_NOT_IMPLMENETED              1
#define ASYNC_ERROR_QUEUE_FULL                   2

/**
 * Async function.
 */
typedef void (*async_func_t)(void *obj_p);

struct async_timer_t {
    struct async_t *async_p;
    unsigned int timeout;
    unsigned int delta;
    async_func_t on_timeout;
    void *obj_p;
    int flags;
    bool stopped;
    struct async_timer_t *next_p;
};

struct async_timer_list_t {
    /* List of timers sorted by expiry time. */
    struct async_timer_t *head_p;
    /* Tail element of list. */
    struct async_timer_t tail;
};

struct async_func_queue_elem_t {
    async_func_t func;
    void *obj_p;
};

struct async_func_queue_t {
    int rdpos;
    int wrpos;
    int length;
    struct async_func_queue_elem_t *list_p;
};

struct async_core_t {
    int tick_in_ms;
    struct async_timer_list_t running_timers;
    struct async_func_queue_t funcs;
};

#include "async/port.h"

/**
 * Initailize given async object.
 */
void async_init(struct async_t *self_p,
                int tick_in_ms);

void async_run_forever(struct async_t *self_p);

/**
 * Destory given instance.
 */
void async_destroy(struct async_t *self_p);

/**
 * Returns once all async functions have been called.
 */
void async_process(struct async_t *self_p);

/**
 * Advance the async time one tick. Should be called periodically.
 */
void async_tick(struct async_t *self_p);

/**
 * Call given function with given argument later.
 */
int async_call(struct async_t *self_p,
               async_func_t func,
               void *obj_p);

/**
 * Initialize given timer. Calls given function with given argument on
 * expiry. Give ASYNC_TIMER_PERIODIC in flags to make the timer
 * periodic.
 */
void async_timer_init(struct async_timer_t *self_p,
                      async_func_t on_timeout,
                      void *obj_p,
                      int flags,
                      struct async_t *async_p);

/**
 * (Re)start given timer with given timeout.
 */
void async_timer_start(struct async_timer_t *self_p,
                       int timeout_ms);

/**
 * Stop given timer. This is a noop if the timer has already been
 * stopped.
 */
void async_timer_stop(struct async_timer_t *self_p);

/**
 * Returns true if given timer is stopped. Returns false if given
 * timer is running or has expired. Can be used in the timeout
 * function to check if given timer has been stopped after it expired.
 */
bool async_timer_is_stopped(struct async_timer_t *self_p);

#endif
