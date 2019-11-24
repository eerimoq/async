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
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include "async.h"
#include "async/linux.h"

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
};

static void fatal_perror(const char *message_p)
{
    perror(message_p);
    exit(1);
}

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

static bool bob_got_response(struct bob_t *self_p)
{
    return (self_p->no_response_count == 0);
}

static void say(const char *text_p)
{
    printf("Bob: %s\n", text_p);
}

static void bob_expect_response(struct bob_t *self_p)
{
    printf("%s: ", &self_p->name[0]);
    fflush(stdout);
    async_timer_start(&self_p->response_timer, 10000);
    self_p->no_response_count = 0;
}

static void bob_say_hello(struct bob_t *self_p)
{
    strcpy(&self_p->name[0], "You");
    say("Hello!");
    self_p->state = state_wait_for_greeting_t;
    bob_expect_response(self_p);
}

static void bob_init(struct bob_t *self_p)
{
    strcpy(&self_p->name[0], "You");
    line_reset(self_p);
    bob_say_hello(self_p);
}

static void handle_timeout(struct async_t *async_p,
                           int timer_fd)
{
    uint64_t value;
    ssize_t res;

    res = read(timer_fd, &value, sizeof(value));

    if (res != sizeof(value)) {
        fatal_perror("read timer");
    }

    async_tick(async_p);
}

static void bob_on_response_timeout(struct bob_t *self_p)
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
            bob_say_hello(self_p);
        }
    }
}

static void bob_ask_for_name(struct bob_t *self_p)
{
    say("What is your name?");
    self_p->state = state_wait_for_name_t;
    bob_expect_response(self_p);
}

static int bob_on_stdin_greeting(struct bob_t *self_p)
{
    int res;

    res = -1;

    if (strcmp(line_get(self_p), "Hi!") == 0) {
        bob_ask_for_name(self_p);
        res = 0;
    }

    return (res);
}

static void bob_ask_for_age(struct bob_t *self_p)
{
    say("How old are you?");
    self_p->state = state_wait_for_age_t;
    bob_expect_response(self_p);
}

static int bob_on_stdin_name(struct bob_t *self_p)
{
    if (bob_got_response(self_p)) {
        strncpy(&self_p->name[0], line_get(self_p), sizeof(self_p->name));
        self_p->name[sizeof(self_p->name) - 1] = '\0';
        bob_ask_for_age(self_p);
    } else {
        bob_ask_for_name(self_p);
    }

    return (0);
}

static int bob_on_stdin_age(struct bob_t *self_p)
{
    if (bob_got_response(self_p)) {
        say("That's it, thanks!\n");
        bob_say_hello(self_p);
    } else {
        bob_ask_for_age(self_p);
    }

    return (0);
}

static void bob_on_stdin(struct bob_t *self_p)
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
        res = bob_on_stdin_greeting(self_p);
        break;

    case state_wait_for_name_t:
        res = bob_on_stdin_name(self_p);
        break;

    case state_wait_for_age_t:
        res = bob_on_stdin_age(self_p);
        break;

    default:
        res = false;
        break;
    }

    line_reset(self_p);

    if (res != 0) {
        say("I don't understand.");
        bob_say_hello(self_p);
    }
}

static void handle_stdin(struct async_t *async_p,
                         struct bob_t *bob_p)
{
    async_call(async_p, (async_func_t)bob_on_stdin, bob_p);
}

static int init_periodic_timer(struct async_t *async_p,
                               int epoll_fd)
{
    int res;
    int timer_fd;
    struct epoll_event event;

    timer_fd = async_linux_create_periodic_timer(async_p);

    if (timer_fd == -1) {
        fatal_perror("async_linux_create_periodic_timer");
    }

    event.events = EPOLLIN;
    event.data.fd = timer_fd;
    res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event);

    if (res == -1) {
        fatal_perror("epoll_ctl timer");
    }

    return (timer_fd);
}

static void init_stdin(int epoll_fd)
{
    int res;
    struct epoll_event event;

    res = fcntl(fileno(stdin),
                F_SETFL,
                fcntl(fileno(stdin), F_GETFL) | O_NONBLOCK);

    if (res == -1) {
        fatal_perror("fcntl");
    }

    event.events = EPOLLIN;
    event.data.fd = fileno(stdin);
    res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fileno(stdin), &event);

    if (res == -1) {
        fatal_perror("epoll_ctl stdin");
    }
}

int main()
{
    struct async_t async;
    int timer_fd;
    int epoll_fd;
    struct epoll_event event;
    int nfds;
    struct bob_t bob;

    async_init(&async, 100);
    async_timer_init(&bob.response_timer,
                     (async_func_t)bob_on_response_timeout,
                     &bob,
                     0,
                     &async);
    bob_init(&bob);

    epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        fatal_perror("epoll_create1");
    }

    timer_fd = init_periodic_timer(&async, epoll_fd);
    init_stdin(epoll_fd);

    while (true) {
        nfds = epoll_wait(epoll_fd, &event, 1, -1);

        if (nfds == 1) {
            if (event.data.fd == timer_fd) {
                handle_timeout(&async, timer_fd);
            } else if (event.data.fd == fileno(stdin)) {
                handle_stdin(&async, &bob);
            }
        }

        async_process(&async);
    }

    return (1);
}
