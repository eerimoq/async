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

#include "async/core.h"
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
                                  struct async_func_queue_elem_t *elems_p,
                                  int length)
{
    self_p->rdpos = 0;
    self_p->wrpos = 0;
    self_p->length = length;
    self_p->list_p = elems_p;
}

static void async_func_queue_destroy(struct async_func_queue_t *self_p)
{
    (void)self_p;
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

static void log_object_print_null(void *log_object_p,
                                  int level,
                                  const char *fmt_p,
                                  ...)
{
    (void)log_object_p;
    (void)level;
    (void)fmt_p;
}

static bool log_object_is_enabled_for_null(void *log_object_p,
                                           int level)
{
    (void)log_object_p;
    (void)level;

    return (false);
}

void async_init(struct async_t *self_p)
{
    self_p->tick_in_ms = 100;
    async_timer_list_init(&self_p->running_timers);
    async_func_queue_init(&self_p->funcs,
                          &self_p->elems[0],
                          ASYNC_FUNC_QUEUE_MAX);
    self_p->log_object.print = log_object_print_null;
    self_p->log_object.is_enabled_for = log_object_is_enabled_for_null;
    self_p->runtime_p = async_runtime_null_create();
}

void async_set_log_object_callbacks(
    struct async_t *self_p,
    async_log_object_print_t log_object_print,
    async_log_object_is_enabled_for_t log_object_is_enabled_for)
{
    if (log_object_print == NULL) {
        log_object_print = log_object_print_null;
    }

    if (log_object_is_enabled_for == NULL) {
        log_object_is_enabled_for = log_object_is_enabled_for_null;
    }

    self_p->log_object.print = log_object_print;
    self_p->log_object.is_enabled_for = log_object_is_enabled_for;
}

void async_set_tick_in_ms(struct async_t *self_p,
                          int tick_in_ms)
{
    self_p->tick_in_ms = tick_in_ms;
}

void async_set_runtime(struct async_t *self_p,
                       struct async_runtime_t *runtime_p)
{
    if (runtime_p == NULL) {
        runtime_p = async_runtime_null_create();
    }

    runtime_p->set_async(runtime_p->obj_p, self_p);
    self_p->runtime_p = runtime_p;
}

void async_destroy(struct async_t *self_p)
{
    async_func_queue_destroy(&self_p->funcs);
}

void async_tick(struct async_t *self_p)
{
    async_timer_list_tick(&self_p->running_timers);
}

void async_process(struct async_t *self_p)
{
    async_func_t func;
    void *obj_p;

    while (true) {
        func = async_func_queue_get(&self_p->funcs, &obj_p);

        if (func == NULL) {
            break;
        }

        func(obj_p);
    }
}

int async_call(struct async_t *self_p, async_func_t func, void *obj_p)
{
    return (async_func_queue_put(&self_p->funcs, func, obj_p));
}

void async_call_threadsafe(struct async_t *self_p,
                           async_threadsafe_func_t func,
                           struct async_threadsafe_data_t *data_p)
{
    self_p->runtime_p->call_threadsafe(self_p->runtime_p->obj_p,
                                       func,
                                       data_p);
}

static void on_complete_default(void *obj_p)
{
    (void)obj_p;
}

int async_call_worker_pool(struct async_t *self_p,
                           async_func_t entry,
                           void *obj_p,
                           async_func_t on_complete)
{
    if (on_complete == NULL) {
        on_complete = on_complete_default;
    }

    return (self_p->runtime_p->call_worker_pool(self_p->runtime_p->obj_p,
                                                entry,
                                                obj_p,
                                                on_complete));
}

void async_run_forever(struct async_t *self_p)
{
    self_p->runtime_p->run_forever(self_p->runtime_p->obj_p);
}
