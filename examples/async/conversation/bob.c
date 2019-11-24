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

static void say(const char *text_p)
{
    printf("Bob: %s\n", text_p);
}

static void expect_response(struct bob_t *self_p)
{
    printf("%s: ", &self_p->name[0]);
    fflush(stdout);
    async_timer_start(&self_p->response_timer, 10000);
    self_p->no_response_count = 0;
}

static void say_hello(struct bob_t *self_p)
{
    strcpy(&self_p->name[0], "You");
    say("Hello!");
    self_p->state = state_wait_for_greeting_t;
    expect_response(self_p);
}

static void on_response_timeout(struct bob_t *self_p)
{
    if (!async_timer_is_stopped(&self_p->response_timer)) {
        self_p->no_response_count++;
        printf("\n");

        if (self_p->no_response_count <= 3) {
            say("Do you hear me?");
            printf("%s: ", &self_p->name[0]);
            fflush(stdout);
            async_timer_start(&self_p->response_timer, 10000);
        } else {
            say_hello(self_p);
        }
    }
}

static void ask_for_name(struct bob_t *self_p)
{
    say("What is your name?");
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
    say("How old are you?");
    self_p->state = state_wait_for_age_t;
    expect_response(self_p);
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
        say("That's it, thanks!\n");
        say_hello(self_p);
    } else {
        ask_for_age(self_p);
    }

    return (0);
}

void bob_init(struct bob_t *self_p, struct async_t *async_p)
{
    async_timer_init(&self_p->response_timer,
                     (async_func_t)on_response_timeout,
                     self_p,
                     0,
                     async_p);
    strcpy(&self_p->name[0], "You");
    line_reset(self_p);
    say_hello(self_p);
}

void bob_on_stdin(struct bob_t *self_p)
{
    char ch;
    ssize_t res;

    res = read(fileno(stdin), &ch, 1);

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
        say("I don't understand.");
        say_hello(self_p);
    }
}
