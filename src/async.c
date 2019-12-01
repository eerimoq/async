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

static bool is_empty(struct async_func_queue_t *self_p)
{
    return (self_p->rdpos == self_p->wrpos);
}

static bool is_full(struct async_func_queue_t *self_p)
{
    return (((self_p->wrpos + 1) % self_p->length) == self_p->rdpos);
}

static void async_func_queue_init(struct async_func_queue_t *self_p,
                                  int length)
{
    self_p->rdpos = 0;
    self_p->wrpos = 0;
    self_p->length = (length + 1);
    self_p->list_p = malloc(sizeof(*self_p->list_p) * self_p->length);
}

static void async_func_queue_destroy(struct async_func_queue_t *self_p)
{
    free(self_p->list_p);
}

static async_func_t async_func_queue_get(struct async_func_queue_t *self_p,
                                         void **obj_pp)
{
    async_func_t func;

    if (is_empty(self_p)) {
        return (NULL);
    }

    func = self_p->list_p[self_p->rdpos].func;
    *obj_pp = self_p->list_p[self_p->rdpos].obj_p;
    self_p->rdpos++;
    self_p->rdpos %= self_p->length;

    return (func);
}

static int async_func_queue_put(struct async_func_queue_t *self_p,
                                async_func_t func,
                                void *obj_p)
{
    if (is_full(self_p)) {
        return (-ASYNC_ERROR_QUEUE_FULL);
    }

    self_p->list_p[self_p->wrpos].func = func;
    self_p->list_p[self_p->wrpos].obj_p = obj_p;
    self_p->wrpos++;
    self_p->wrpos %= self_p->length;

    return (0);
}

void async_destroy(struct async_t *self_p)
{
    async_func_queue_destroy(&self_p->core.funcs);
}

void async_run_until_complete(struct async_t *self_p)
{
    async_func_t func;
    void *obj_p;

    while (true) {
        func = async_func_queue_get(&self_p->core.funcs, &obj_p);

        if (func == NULL) {
            break;
        }

        func(obj_p);
    }
}

void async_tick(struct async_t *self_p)
{
    async_timer_list_tick(&self_p->core.running_timers);
}

int async_call(struct async_t *self_p, async_func_t func, void *obj_p)
{
    return (async_func_queue_put(&self_p->core.funcs, func, obj_p));
}

void async_core_init(struct async_core_t *self_p,
                     int tick_in_ms)
{
    self_p->tick_in_ms = tick_in_ms;
    async_timer_list_init(&self_p->running_timers);
    async_func_queue_init(&self_p->funcs, 32);
}
