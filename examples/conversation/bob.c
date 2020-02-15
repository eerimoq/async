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
#include <unistd.h>
#include <string.h>
#include "bob.h"

static void line_append(struct bob_t *self_p, char ch)
{
    if (ch == '\n') {
        self_p->line.complete = true;
    } else if (self_p->line.length < sizeof(self_p->line.buf)) {
        self_p->line.buf[self_p->line.length] = ch;
        self_p->line.length++;
        self_p->line.buf[self_p->line.length] = '\0';
    }
}

static bool line_is_complete(struct bob_t *self_p)
{
    return (self_p->line.complete);
}

static char *line_get(struct bob_t *self_p)
{
    return (&self_p->line.buf[0]);
}

static void line_reset(struct bob_t *self_p)
{
    self_p->line.length = 0;
    self_p->line.buf[self_p->line.length] = '\0';
    self_p->line.complete = false;
}

static bool got_response(struct bob_t *self_p)
{
    return (self_p->no_response_count == 0);
}

static void say(struct bob_t *self_p, const char *text_p)
{
    async_channel_write(self_p->channel_p, "Bob: ", 5);
    async_channel_write(self_p->channel_p, text_p, strlen(text_p));
    async_channel_write(self_p->channel_p, "\n", 1);
}

static void print_prompt(struct bob_t *self_p)
{
    async_channel_write(self_p->channel_p,
                        &self_p->name[0],
                        strlen(&self_p->name[0]));
    async_channel_write(self_p->channel_p, ": ", 2);
}

static void expect_response(struct bob_t *self_p)
{
    print_prompt(self_p);
    async_timer_start(&self_p->response_timer);
    self_p->no_response_count = 0;
}

static void say_hello(struct bob_t *self_p)
{
    strcpy(&self_p->name[0], "You");
    say(self_p, "Hello!");
    self_p->state = state_wait_for_greeting_t;
    expect_response(self_p);
}

static void on_response_timeout(struct bob_t *self_p)
{
    self_p->no_response_count++;
    async_channel_write(self_p->channel_p, "\n", 1);

    if (self_p->no_response_count <= 3) {
        say(self_p, "Do you hear me?");
        print_prompt(self_p);
        async_timer_start(&self_p->response_timer);
    } else {
        say_hello(self_p);
    }
}

static void ask_for_name(struct bob_t *self_p)
{
    say(self_p, "What is your name?");
    self_p->state = state_wait_for_name_t;
    expect_response(self_p);
}

static int on_stdin_greeting(struct bob_t *self_p)
{
    int res;

    res = -1;

    if (strcmp(line_get(self_p), "Hi!") == 0) {
        ask_for_name(self_p);
        res = 0;
    }

    return (res);
}

static void ask_for_age(struct bob_t *self_p)
{
    say(self_p, "How old are you?");
    self_p->state = state_wait_for_age_t;
    expect_response(self_p);
}

static void on_opened(struct bob_t *self_p, int res)
{
    if (res == 0) {
        say_hello(self_p);
    } else {
        say(self_p, "Open failed!");
    }
}

static int on_stdin_name(struct bob_t *self_p)
{
    if (got_response(self_p)) {
        strncpy(&self_p->name[0], line_get(self_p), sizeof(self_p->name));
        self_p->name[sizeof(self_p->name) - 1] = '\0';
        ask_for_age(self_p);
    } else {
        ask_for_name(self_p);
    }

    return (0);
}

static int on_stdin_age(struct bob_t *self_p)
{
    if (got_response(self_p)) {
        say(self_p, "That's it, thanks!\n");
        say_hello(self_p);
    } else {
        ask_for_age(self_p);
    }

    return (0);
}

static void on_input(struct bob_t *self_p)
{
    char ch;
    ssize_t res;

    res = async_channel_read(self_p->channel_p, &ch, 1);

    if (res == 1) {
        line_append(self_p, ch);
    }

    if (!line_is_complete(self_p)) {
        return;
    }

    switch (self_p->state) {

    case state_wait_for_greeting_t:
        res = on_stdin_greeting(self_p);
        break;

    case state_wait_for_name_t:
        res = on_stdin_name(self_p);
        break;

    case state_wait_for_age_t:
        res = on_stdin_age(self_p);
        break;

    default:
        res = false;
        break;
    }

    line_reset(self_p);

    if (res != 0) {
        say(self_p, "I don't understand.");
        say_hello(self_p);
    }
}

void bob_init(struct bob_t *self_p,
              struct async_channel_t *channel_p,
              struct async_t *async_p)
{
    async_timer_init(&self_p->response_timer,
                     (async_timer_timeout_t)on_response_timeout,
                     self_p,
                     10000,
                     0,
                     async_p);
    strcpy(&self_p->name[0], "You");
    line_reset(self_p);
    async_channel_set_on(channel_p,
                         (async_channel_opened_t)on_opened,
                         NULL,
                         (async_func_t)on_input,
                         self_p);
    async_channel_open(channel_p);
    self_p->channel_p = channel_p;
}
