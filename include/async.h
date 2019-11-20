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

#ifndef ASYNC_H
#define ASYNC_H

#include <stdbool.h>
#include <stdlib.h>

#define ASYNC_TIMER_PERIODIC                     (1 << 0)

#define ASYNC_ERROR_NOT_IMPLMENETED              1
#define ASYNC_ERROR_QUEUE_FULL                   2

/**
 * Create a unique identifier.
 */
#define ASYNC_UID(name)                         \
    struct async_uid_t name = {                 \
        .name_p = #name                         \
    }

struct async_uid_t;
struct async_task_t;

typedef void (*async_task_on_message_t)(struct async_task_t *self_p,
                                        struct async_uid_t *uid_p,
                                        void *message_p);

typedef void (*async_message_on_free_t)(void *message_p);

struct async_uid_t {
    const char *name_p;
};

struct async_message_header_t {
    struct async_uid_t *uid_p;
    int count;
    async_message_on_free_t on_free;
};

struct async_timer_t {
    struct async_t *async_p;
    unsigned int timeout;
    unsigned int delta;
    struct async_uid_t *message_p;
    struct async_task_t *task_p;
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

struct async_queue_message_t {
    struct async_task_t *receiver_p;
    struct async_message_header_t *message_p;
};

struct async_queue_t {
    int rdpos;
    int wrpos;
    int length;
    struct async_queue_message_t *messages_p;
};

struct async_t {
    int tick_in_ms;
    struct async_timer_list_t running_timers;
    struct async_queue_t messages;
};

struct async_task_t {
    struct async_t *async_p;
    async_task_on_message_t on_message;
};

/**
 * Initailize given async object. The buffer is not used yet in the
 * current implementation. Using malloc() and free() instead.
 */
void async_init(struct async_t *self_p,
                int tick_in_ms,
                void *buf_p,
                size_t size);

/**
 * Returns once all messages have been processed.
 */
void async_process(struct async_t *self_p);

/**
 * Advance the async time one tick. Should be called periodically.
 */
void async_tick(struct async_t *self_p);

/**
 * Create a queue of given length. on_message is called to process
 * received messages.
 */
void async_task_init(struct async_task_t *self_p,
                     struct async_t *async_p,
                     async_task_on_message_t on_message);

/**
 * Send given message to given task.
 */
int async_send(struct async_task_t *receiver_p, void *message_p);

/**
 * Allocate a message with given id and size. The size may be zero.
 */
void *async_message_alloc(struct async_t *async_p,
                          struct async_uid_t *uid_p,
                          size_t size);

/**
 * Set the on free callback. Must be called before putting the message
 * on a queue or broadcasting it on a bus.
 */
void async_message_set_on_free(void *message_p,
                               async_message_on_free_t on_free);

/**
 * Free given message.
 */
void async_message_free(void *message_p);

/**
 * Initialize given timer. Sends a message with given id to given task
 * on expiry. Give ASYNC_TIMER_PERIODIC in flags to make the timer
 * periodic.
 */
void async_timer_init(struct async_timer_t *self_p,
                      struct async_t *async_p,
                      int timeout_ms,
                      struct async_uid_t *message_p,
                      struct async_task_t *task_p,
                      int flags);

/**
 * (Re)start given timer.
 */
void async_timer_start(struct async_timer_t *self_p);

/**
 * Stop given timer. This is a noop if the timer has already been
 * stopped.
 */
void async_timer_stop(struct async_timer_t *self_p);

/**
 * Returns true if given timer is stopped. Returns false if given
 * timer is running or has expired. Can be used to check if a timeout
 * message is recieved before or after given timer has been stopped.
 */
bool async_timer_is_stopped(struct async_timer_t *self_p);

#endif
