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

#ifndef ASYNC_SHELL_H
#define ASYNC_SHELL_H

#include "async.h"

typedef int (*async_shell_command_t)(int argc, const char *argv[]);

#define ASYNC_SHELL_COMMAND_MAX                              256

struct async_shell_command_t {
    const char *name_p;
    const char *description_p;
    async_shell_command_callback_t callback;
};

struct async_shell_history_elem_t {
    struct history_elem_t *next_p;
    struct history_elem_t *prev_p;
    char buf[1];
};

struct async_shell_line_t {
    char buf[ASYNC_SHELL_COMMAND_MAX];
    int length;
    int cursor;
};

struct async_shell_t {
    struct async_shell_line_t line;
    struct async_shell_line_t prev_line;
    bool carriage_return_received;
    bool newline_received;
    struct {
        struct async_shell_history_elem_t *head_p;
        struct async_shell_history_elem_t *tail_p;
        int length;
        struct async_shell_history_elem_t *current_p;
        struct async_shell_line_t pattern;
        struct async_shell_line_t match;
        /* Command line when first UP was pressed. */
        struct async_shell_line_t line;
        bool line_valid;
    } history;
    int number_of_commands;
    struct async_shell_command_t *commands_p;
    struct async_stream_t *stream_p;
    struct async_t *async_p;
};

/**
 * Initialize given shell. Commands may be registered after this
 * function has been called.
 */
void async_shell_init(struct async_shell_t *self_p,
                      struct async_stream_t *stream_p,
                      struct async_t *async_p);

/**
 * Start given shell. No commands may be registered after this
 * function has been called.
 */
void async_shell_start(struct async_shell_t *self_p);

/**
 * Stop given shell.
 */
void async_shell_stop(struct async_shell_t *self_p);

/**
 * Must be called before async_shell_start().
 */
void async_shell_register_command(struct async_shell_t *self_p,
                                  const char *name_p,
                                  const char *description_p,
                                  async_shell_command_t callback);

#endif
