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

#ifndef BOB_H
#define BOB_H

#include <stdbool.h>
#include "async.h"

enum state_t {
    state_init_t = 0,
    state_wait_for_greeting_t,
    state_wait_for_name_t,
    state_wait_for_age_t
};

struct bob_t {
    enum state_t state;
    struct async_timer_t response_timer;
    int no_response_count;
    char name[32];
    struct {
        char buf[128];
        size_t length;
        bool complete;
    } line;
    struct async_channel_t *channel_p;
};

void bob_init(struct bob_t *self_p,
              struct async_channel_t *channel_p,
              struct async_t *async_p);

/**
 * Call when there is data available on stdin.
 */
void bob_on_stdin(struct bob_t *self_p);

#endif
