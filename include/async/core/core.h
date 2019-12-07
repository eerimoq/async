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

/* Error codes. */
#define ASYNC_ERROR_NOT_IMPLMENETED              1
#define ASYNC_ERROR_QUEUE_FULL                   2

#define ASYNC_FUNC_QUEUE_MAX                     (32 + 1)

#define async_offsetof(type, member) ((size_t) &((type *)0)->member)

#define async_container_of(ptr, type, member)                   \
    ((type *) ((char *)(ptr) - async_offsetof(type, member)))

struct async_runtime_t;

/**
 * Async function.
 */
typedef void (*async_func_t)(void *obj_p);

struct async_timer_t;

typedef void (*async_timer_timeout_t)(struct async_timer_t *self_p);

struct async_timer_t {
    struct async_t *async_p;
    unsigned int initial;
    unsigned int repeat;
    unsigned int initial_ticks;
    unsigned int repeat_ticks;
    unsigned int delta;
    async_timer_timeout_t on_timeout;
    int number_of_outstanding_timeouts;
    int number_of_timeouts_to_ignore;
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

struct async_t {
    int tick_in_ms;
    struct async_timer_list_t running_timers;
    struct async_func_queue_t funcs;
    struct async_func_queue_elem_t elems[ASYNC_FUNC_QUEUE_MAX];
    struct async_runtime_t *runtime_p;
};

/**
 * Initailize given async object with a null runtime. Set a runtime
 * with async_set_runtime() if needed.
 */
void async_init(struct async_t *self_p);

/**
 * Set the tick duration in milliseconds for given async object.
 */
void async_set_tick_in_ms(struct async_t *self_p, int tick_in_ms);

/**
 * Set the runtime for given async object. The default runtime exits
 * the program if used.
 */
void async_set_runtime(struct async_t *self_p,
                       struct async_runtime_t *runtime_p);

/**
 * Destory given instance.
 */
void async_destroy(struct async_t *self_p);

/**
 * Advance the async time one tick. Should be called periodically.
 */
void async_tick(struct async_t *self_p);

/**
 * Returns once all async functions have been called.
 */
void async_process(struct async_t *self_p);

/**
 * Call given function with given argument later.
 */
int async_call(struct async_t *self_p,
               async_func_t func,
               void *obj_p);

/**
 * Run given async object forever. This function never returns.
 */
void async_run_forever(struct async_t *self_p);

/**
 * Initialize given timer with given initial and repeat timeouts in
 * milliseconds. Calls on_timeout() on expiry.
 */
void async_timer_init(struct async_timer_t *self_p,
                      async_timer_timeout_t on_timeout,
                      unsigned int initial,
                      unsigned int repeat,
                      struct async_t *async_p);

/**
 * Set the initial timeout in miliseconds.
 */
void async_timer_set_initial(struct async_timer_t *self_p,
                             unsigned int initial);

/**
 * Get the initial timeout in miliseconds.
 */
unsigned int async_timer_get_initial(struct async_timer_t *self_p);

/**
 * Set the repeat timeout in miliseconds.
 */
void async_timer_set_repeat(struct async_timer_t *self_p,
                            unsigned int repeat);

/**
 * Get the repeat timeout in miliseconds.
 */
unsigned int async_timer_get_repeat(struct async_timer_t *self_p);

/**
 * (Re)start given timer.
 */
void async_timer_start(struct async_timer_t *self_p);

/**
 * Stop given timer. This is a noop if the timer has already been
 * stopped.
 */
void async_timer_stop(struct async_timer_t *self_p);

#endif
