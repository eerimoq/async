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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <ctype.h>
#include "async/shell.h"
#include "hf.h"

#define PROMPT                                  "$ "

#define COMMAND_MAX                              256

#define MAXIMUM_HISTORY_LENGTH                    64

#define TAB                                     '\t'
#define CARRIAGE_RETURN                         '\r'
#define NEWLINE                                 '\n'
#define BACKSPACE                                  8
#define DELETE                                   127
#define CTRL_A                                     1
#define CTRL_E                                     5
#define CTRL_D                                     4
#define CTRL_K                                    11
#define CTRL_T                                    20
#define CTRL_G                                     7
#define ALT                                       27

static void history_init(struct async_shell_t *self_p);
static void show_line(struct async_shell_t *self_p);

static void output(struct async_shell_t *self_p, const char *string_p)
{
    async_channel_write(self_p->channel_p, string_p, strlen(string_p));
}

static void output_int(struct async_shell_t *self_p, int value)
{
    char buf[32];

    sprintf(&buf[0], "%d", value);
    output(self_p, &buf[0]);
}

static void output_sds(struct async_shell_t *self_p,
                       const char *value_1_p,
                       int value_2,
                       const char *value_3_p)
{
    output(self_p, value_1_p);
    output_int(self_p, value_2);
    output(self_p, value_3_p);
}

static void output_control(struct async_shell_t *self_p,
                           int value_1,
                           const char *value_2_p)
{
    output_sds(self_p, "\x1b[", value_1, value_2_p);
}

static int xgetc(struct async_shell_t *self_p)
{
    char ch;
    size_t size;

    size = async_channel_read(self_p->channel_p, &ch, sizeof(ch));

    if (size == 0) {
        return (-EAGAIN);
    }

    return ((int)ch);
}

static void print_prompt(struct async_shell_t *self_p)
{
    output(self_p, PROMPT);
}

static struct async_shell_command_t *find_command(struct async_shell_t *self_p,
                                                  const char *name_p)
{
    struct async_shell_command_t *command_p;

    command_p = self_p->commands_p;

    while (command_p != NULL) {
        if (strcmp(name_p, command_p->name_p) == 0) {
            return (command_p);
        }

        command_p = command_p->next_p;
    }

    return (NULL);
}

/**
 * Parse one argument from given string. An argument must be in quotes
 * if it contains spaces.
 */
static char *parse_argument(char *line_p, const char **begin_pp)
{
    bool in_quote;

    in_quote = false;
    *begin_pp = line_p;

    while (*line_p != '\0') {
        if (*line_p == '\\') {
            if (line_p[1] == '\"') {
                /* Remove the \. */
                memmove(line_p, &line_p[1], strlen(&line_p[1]) + 1);
            }
        } else {
            if (in_quote) {
                if (*line_p == '\"') {
                    /* Remove the ". */
                    memmove(line_p, &line_p[1], strlen(&line_p[1]) + 1);
                    in_quote = false;
                    line_p--;
                }
            } else {
                if (*line_p == '\"') {
                    /* Remove the ". */
                    memmove(line_p, &line_p[1], strlen(&line_p[1]) + 1);
                    in_quote = true;
                    line_p--;
                } else if (*line_p == ' ') {
                    *line_p = '\0';
                    line_p++;
                    break;
                }
            }
        }

        line_p++;
    }

    if (in_quote) {
        line_p =  NULL;
    }

    return (line_p);
}

static int parse_command(char *line_p, const char *argv[])
{
    int argc;

    /* Remove white spaces at the beginning and end of the string. */
    line_p = hf_strip(line_p, NULL);
    argc = 0;

    /* Command string missing. */
    if (strlen(line_p) == 0) {
        return (-1);
    }

    while (*line_p != '\0') {
        /* Too many arguemnts? */
        if (argc == 32) {
            return (-1);
        }

        /* Remove white spaces before the next argument. */
        line_p = hf_strip(line_p, NULL);

        if ((line_p = parse_argument(line_p, &argv[argc++])) == NULL) {
            return (-1);
        }
    }

    return (argc);
}

static int execute_command(struct async_shell_t *self_p,
                           char *line_p)
{
    struct async_shell_command_t *command_p;
    const char *name_p;
    int res;
    const char *argv[32];
    int argc;

    argc = parse_command(line_p, &argv[0]);

    if (argc < 1) {
        return (-1);
    }

    name_p = argv[0];

    if (name_p == NULL) {
        name_p = "";
    }

    command_p = find_command(self_p, name_p);

    if (command_p != NULL) {
        res = command_p->callback(self_p, argc, &argv[0]);
    } else {
        output(self_p, name_p);
        output(self_p, ": command not found\n");
        res = -1;
    }

    return (res);
}

static bool is_comment(const char *line_p)
{
    return (*line_p == '#');
}

static void line_init(struct async_shell_line_t *self_p)
{
    self_p->buf[0] = '\0';
    self_p->cursor = 0;
    self_p->length = 0;
}

static bool line_insert(struct async_shell_line_t *self_p,
                        int ch)
{
    /* Buffer full? */
    if (self_p->length == COMMAND_MAX - 1) {
        return (false);
    }

    /* Move the string, including the NULL termination, one step to
       the right and insert the new character. */
    memmove(&self_p->buf[self_p->cursor + 1],
            &self_p->buf[self_p->cursor],
            self_p->length - self_p->cursor + 1);
    self_p->buf[self_p->cursor++] = ch;
    self_p->length++;

    return (true);
}

static void line_insert_string(struct async_shell_line_t *self_p,
                               char *str_p)
{
    while (*str_p != '\0') {
        if (!line_insert(self_p, *str_p)) {
            break;
        }

        str_p++;
    }
}

static void line_delete(struct async_shell_line_t *self_p)
{
    /* End of buffer? */
    if (self_p->cursor == self_p->length) {
        return;
    }

    /* Move the string, including the NULL termination, one step to
       the left to overwrite the deleted character. */
    memmove(&self_p->buf[self_p->cursor],
            &self_p->buf[self_p->cursor + 1],
            self_p->length - self_p->cursor);
    self_p->length--;
}

static int line_peek(struct async_shell_line_t *self_p)
{
    return (self_p->buf[self_p->cursor]);
}

static void line_truncate(struct async_shell_line_t *self_p)
{
    self_p->length = self_p->cursor;
    self_p->buf[self_p->length] = '\0';
}

static bool line_is_empty(struct async_shell_line_t *self_p)
{
    return (self_p->length == 0);
}

static char *line_get_buf(struct async_shell_line_t *self_p)
{
    return (self_p->buf);
}

static int line_get_length(struct async_shell_line_t *self_p)
{
    return (self_p->length);
}

static bool line_seek(struct async_shell_line_t *self_p, int pos)
{
    if (pos < 0) {
        if ((self_p->cursor + pos) < 0) {
            return (false);
        }
    } else {
        if ((self_p->cursor + pos) > self_p->length) {
            return (false);
        }
    }

    self_p->cursor += pos;

    return (true);
}

static int line_get_cursor(struct async_shell_line_t *self_p)
{
    return (self_p->cursor);
}

static void line_seek_begin(struct async_shell_line_t *self_p)
{
    self_p->cursor = 0;
}

static void line_seek_end(struct async_shell_line_t *self_p)
{
    self_p->cursor = self_p->length;
}

static int command_help(struct async_shell_t *self_p,
                        int argc,
                        const char *argv[])
{
    (void)argc;
    (void)argv;

    char buf[128];
    struct async_shell_command_t *command_p;

    output(self_p,
           "Cursor movement\n"
           "\n"
           "         LEFT   Go left one character.\n"
           "        RIGHT   Go right one character.\n"
           "  HOME/Ctrl+A   Go to the beginning of the line.\n"
           "   END/Ctrl+E   Go to the end of the line.\n"
           "\n"
           "Edit\n"
           "\n"
           "        Alt+D   Delete the word at the cursor.\n"
           "       Ctrl+D   Delete the chracter at the cursor.\n"
           "       Ctrl+K   Cut the line from cursor to end.\n"
           "       Ctrl+T   Swap the last two characters before the cursor "
           "(typo).\n"
           "          TAB   Tab completion for command names.\n"
           "    BACKSPACE   Delete the character before the cursor.\n"
           "\n"
           "History\n"
           "\n"
           "           UP   Previous command.\n"
           "         DOWN   Next command.\n"
           "\n"
           "Commands\n"
           "\n");

    command_p = self_p->commands_p;

    while (command_p != NULL) {
        snprintf(&buf[0],
                 sizeof(buf),
                 "%13s   %s\n",
                 command_p->name_p,
                 command_p->description_p);
        output(self_p, &buf[0]);
        command_p = command_p->next_p;
    }

    return (0);
}

static int command_history(struct async_shell_t *self_p,
                           int argc,
                           const char *argv[])
{
    (void)argc;
    (void)argv;

    struct async_shell_history_elem_t *current_p;
    int i;

    current_p = self_p->history.head_p;
    i = 1;

    while (current_p != NULL) {
        output_int(self_p, i);
        output(self_p, ": ");
        output(self_p, current_p->buf);
        output(self_p, "\n");
        current_p = current_p->next_p;
        i++;
    }

    return (0);
}

static void history_init(struct async_shell_t *self_p)
{
    self_p->history.head_p = NULL;
    self_p->history.tail_p = NULL;
    self_p->history.length = 0;
    self_p->history.current_p = NULL;
    line_init(&self_p->history.line);
    self_p->history.line_valid = false;
}

static int history_append(struct async_shell_t *self_p,
                          char *command_p)
{
    struct async_shell_history_elem_t *elem_p, *head_p;

    /* Do not append if the command already is at the end of the
       list. */
    if (self_p->history.tail_p != NULL) {
        if (strcmp(self_p->history.tail_p->buf, command_p) == 0) {
            return (0);
        }
    }

    if (self_p->history.length == MAXIMUM_HISTORY_LENGTH) {
        head_p = self_p->history.head_p;

        /* Remove the head element from the list. */
        if (head_p == self_p->history.tail_p) {
            self_p->history.head_p = NULL;
            self_p->history.tail_p = NULL;
        } else {
            self_p->history.head_p = head_p->next_p;
            head_p->next_p->prev_p = NULL;
        }

        free(head_p);
        self_p->history.length--;
    }

    /* Allocate memory. */
    elem_p = malloc(sizeof(*elem_p) + strlen(command_p) + 1);

    if (elem_p != NULL) {
        strcpy(elem_p->buf, command_p);

        /* Append the command to the command history list. */
        elem_p->next_p = NULL;

        if (self_p->history.head_p == NULL) {
            elem_p->prev_p = NULL;
            self_p->history.head_p = elem_p;
        } else {
            elem_p->prev_p = self_p->history.tail_p;
            elem_p->prev_p->next_p = elem_p;
        }

        self_p->history.tail_p = elem_p;
        self_p->history.length++;
    }

    return (0);
}

/**
 * Find the previous element, if any.
 */
static char *history_get_previous_command(struct async_shell_t *self_p)
{
    if (self_p->history.current_p == self_p->history.head_p) {
        return (NULL);
    } else if (self_p->history.current_p == NULL) {
        self_p->history.current_p = self_p->history.tail_p;

        /* Save the current command to be able to restore it when DOWN
           is pressed. */
        self_p->history.line = self_p->line;
        self_p->history.line_valid = true;
    } else if (self_p->history.current_p != self_p->history.head_p) {
        self_p->history.current_p = self_p->history.current_p->prev_p;
    }

    if (self_p->history.current_p != NULL) {
        return (self_p->history.current_p->buf);
    } else {
        return (NULL);
    }
}

/**
 * Find the next element, if any.
 */
static char *history_get_next_command(struct async_shell_t *self_p)
{
    if (self_p->history.current_p != NULL) {
        self_p->history.current_p = self_p->history.current_p->next_p;
    }

    if (self_p->history.current_p != NULL) {
        return (self_p->history.current_p->buf);
    } else if (self_p->history.line_valid) {
        self_p->history.line_valid = false;

        return (line_get_buf(&self_p->history.line));
    } else {
        return (NULL);
    }
}

static void history_reset_current(struct async_shell_t *self_p)
{
    self_p->history.current_p = NULL;
}

static void auto_complete_command(struct async_shell_t *self_p)
{
    char next_char;
    bool mismatch;
    int size;
    struct async_shell_command_t *first_match_p;
    bool completion_happend;
    char *line_p;
    struct async_shell_command_t *command_p;

    line_p = line_get_buf(&self_p->line);
    size = line_get_length(&self_p->line);

    /* Find the first command matching given line. */
    first_match_p = NULL;
    command_p = self_p->commands_p;

    while (command_p != NULL) {
        if (strncmp(command_p->name_p, line_p, size) == 0) {
            first_match_p = command_p;
            break;
        }

        command_p = command_p->next_p;
    }

    /* No command matching the line. */
    if (first_match_p == NULL) {
        return;
    }

    /* Auto-complete given line. Compare the next character each
       iteration. */
    completion_happend = false;

    while (true) {
        mismatch = false;
        next_char = first_match_p->name_p[size];

        /* It's a match if all commands matching line has the same
           next character. */
        command_p = first_match_p;

        while ((command_p != NULL)
               && (strncmp(&command_p->name_p[0], line_p, size) == 0)) {
            if (command_p->name_p[size] != next_char) {
                mismatch = true;
                break;
            }

            command_p = command_p->next_p;
        }

        /* This character mismatch? */
        if (mismatch) {
            break;
        }

        completion_happend = true;

        /* Append a space on full match. */
        if (next_char == '\0') {
            line_insert(&self_p->line, ' ');
            break;
        }

        line_insert(&self_p->line, next_char);
        size++;
    }

    /* Print all alternatives if no completion happened. */
    if (!completion_happend) {
        output(self_p, "\n");
        command_p = first_match_p;

        while ((command_p != NULL)
               && (strncmp(command_p->name_p, line_p, size) == 0)) {
            output(self_p, command_p->name_p);
            output(self_p, "\n");
            command_p = command_p->next_p;
        }

        print_prompt(self_p);
        output(self_p, line_get_buf(&self_p->line));
    }
}

static void handle_tab(struct async_shell_t *self_p)
{
    auto_complete_command(self_p);
}

static void handle_carrige_return(struct async_shell_t *self_p)
{
    self_p->carriage_return_received = true;
}

static void handle_newline(struct async_shell_t *self_p)
{
    self_p->newline_received = true;
}

/**
 * BACKSPACE Delete the character before the cursor.
 */
static void handle_backspace(struct async_shell_t *self_p)
{
    if (line_seek(&self_p->line, -1)) {
        line_delete(&self_p->line);
    }
}

/**
 * Ctrl+A Go to the beginning of the line.
 */
static void handle_ctrl_a(struct async_shell_t *self_p)
{
    line_seek_begin(&self_p->line);
}

/**
 * Ctrl+E Go to the end of the line.
 */
static void handle_ctrl_e(struct async_shell_t *self_p)
{
    line_seek_end(&self_p->line);
}

/**
 * Ctrl+D Delete the chracter at the cursor.
 */
static void handle_ctrl_d(struct async_shell_t *self_p)
{
    line_delete(&self_p->line);
}

/**
 * Ctrl+K Cut the line from cursor to end.
 */
static void handle_ctrl_k(struct async_shell_t *self_p)
{
    line_truncate(&self_p->line);
}

/**
 * Ctrl+T Swap the last two characters before the cursor (typo).
 */
static void handle_ctrl_t(struct async_shell_t *self_p)
{
    int ch;
    int cursor;

    /* Is a swap possible? */
    cursor = line_get_cursor(&self_p->line);

    /* Cannot swap if the cursor is at the beginning of the line. */
    if (cursor == 0) {
        return;
    }

    /* Cannot swap if there are less than two characters. */
    if (line_get_length(&self_p->line) < 2) {
        return;
    }

    /* Move the cursor to the second character. */
    if (cursor == line_get_length(&self_p->line)) {
        line_seek(&self_p->line, -1);
    }

    /* Swap the two characters. */
    ch = line_peek(&self_p->line);
    line_delete(&self_p->line);
    line_seek(&self_p->line, -1);
    line_insert(&self_p->line, ch);
    line_seek(&self_p->line, 1);
}

/**
 * ALT.
 */
static int handle_alt(struct async_shell_t *self_p)
{
    self_p->command_reader_state = async_shell_command_reader_state_alt_t;

    return (-1);
}

static void handle_other(struct async_shell_t *self_p,
                         char ch)
{
    line_insert(&self_p->line, ch);
}

/**
 * Show updated line to the user and update the cursor to its new
 * position.
 */
static void show_line(struct async_shell_t *self_p)
{
    int i;
    int cursor;
    int new_cursor;
    int length;
    int new_length;
    int min_length;

    cursor = line_get_cursor(&self_p->prev_line);
    length = line_get_length(&self_p->prev_line);
    new_length = line_get_length(&self_p->line);
    new_cursor = line_get_cursor(&self_p->line);
    min_length = MIN(line_get_length(&self_p->prev_line), new_length);

    /* Was the line edited? */
    if (strcmp(line_get_buf(&self_p->line),
               line_get_buf(&self_p->prev_line)) != 0) {
        /* Only output the change if the last part of the string
           shall be deleted. */
        if ((strncmp(line_get_buf(&self_p->line),
                     line_get_buf(&self_p->prev_line),
                     min_length) == 0)
            && (new_cursor == new_length)) {
            if (length < new_length) {
                /* New character. */
                output(self_p, &line_get_buf(&self_p->line)[cursor]);
            } else {
                /* Move the cursor to the end of the old line. */
                for (i = cursor; i < length; i++) {
                    output(self_p, " ");
                }

                /* Backspace. */
                for (i = new_length; i < length; i++) {
                    output(self_p, "\x08 \x08");
                }
            }
        } else {
            if (cursor > 0) {
                output_control(self_p, cursor, "D");
            }

            output(self_p, "\x1b[K");
            output(self_p, line_get_buf(&self_p->line));

            if (new_cursor < new_length) {
                output_control(self_p, new_length - new_cursor, "D");
            }
        }
    } else if (cursor < new_cursor) {
        output_control(self_p, new_cursor - cursor, "C");
    } else if (new_cursor < cursor) {
        output_control(self_p, cursor - new_cursor, "D");
    }
}

/**
 * Execute the current line.
 */
static int execute_line(struct async_shell_t *self_p)
{
    if (self_p->carriage_return_received) {
        output(self_p, "\r");
    }

    output(self_p, "\n");

    /* Append the command to the history. */
    if (!line_is_empty(&self_p->line)) {
        history_append(self_p, line_get_buf(&self_p->line));
    }

    history_reset_current(self_p);

    return (line_get_length(&self_p->line));
}

static int read_command_init(struct async_shell_t *self_p)
{
    line_init(&self_p->line);
    self_p->carriage_return_received = false;
    self_p->newline_received = false;
    self_p->command_reader_state = async_shell_command_reader_state_read_t;

    return (-1);
}

static int read_command_alt(struct async_shell_t *self_p)
{
    int ch;

    ch = xgetc(self_p);

    if (ch == -EAGAIN) {
        return (ch);
    }

    switch (ch) {

    case 'd':
        /* Alt+D Delete the word at the cursor. */
        while (isblank((int)line_peek(&self_p->line))) {
            line_delete(&self_p->line);
        }

        while (!isblank((int)line_peek(&self_p->line))
               && (line_peek(&self_p->line) != '\0')) {
            line_delete(&self_p->line);
        }

        show_line(self_p);
        self_p->command_reader_state = async_shell_command_reader_state_read_t;
        break;

    case 'O':
        self_p->command_reader_state = async_shell_command_reader_state_alt_o_t;
        break;

    case '[':
        self_p->command_reader_state = async_shell_command_reader_state_alt_bracket_t;
        break;

    default:
        self_p->command_reader_state = async_shell_command_reader_state_read_t;
        break;
    }

    return (-1);
}

static int read_command_alt_o(struct async_shell_t *self_p)
{
    int ch;

    ch = xgetc(self_p);

    if (ch == -EAGAIN) {
        return (ch);
    }

    switch (ch) {

    case 'H':
        /* HOME. */
        line_seek_begin(&self_p->line);
        break;

    case 'F':
        /* END. */
        line_seek_end(&self_p->line);
        break;

    default:
        break;
    }

    show_line(self_p);
    self_p->command_reader_state = async_shell_command_reader_state_read_t;

    return (-1);
}

static int read_command_alt_bracket(struct async_shell_t *self_p)
{
    int ch;
    char *buf_p;

    ch = xgetc(self_p);

    if (ch == -EAGAIN) {
        return (ch);
    }

    switch (ch) {

    case 'A':
    case 'B':
        if (ch == 'A') {
            /* UP Previous command. */
            buf_p = history_get_previous_command(self_p);
        } else {
            /* DOWN Next command. */
            buf_p = history_get_next_command(self_p);
        }

        if (buf_p != NULL) {
            line_init(&self_p->line);
            line_insert_string(&self_p->line, buf_p);
        }

        break;

    case 'C':
        /* RIGHT Go right on character. */
        line_seek(&self_p->line, 1);
        break;

    case 'D':
        /* LEFT Go left one character. */
        line_seek(&self_p->line, -1);
        break;

    default:
        break;
    }

    show_line(self_p);
    self_p->command_reader_state = async_shell_command_reader_state_read_t;

    return (-1);
}

static int read_command_any(struct async_shell_t *self_p)
{
    int ch;

    ch = xgetc(self_p);

    if (ch == -EAGAIN) {
        return (ch);
    }

    /* Save current line. */
    self_p->prev_line = self_p->line;

    switch (ch) {

    case TAB:
        handle_tab(self_p);
        break;

    case CARRIAGE_RETURN:
        handle_carrige_return(self_p);
        break;

    case NEWLINE:
        handle_newline(self_p);
        break;

    case DELETE:
    case BACKSPACE:
        handle_backspace(self_p);
        break;

    case CTRL_A:
        handle_ctrl_a(self_p);
        break;

    case CTRL_E:
        handle_ctrl_e(self_p);
        break;

    case CTRL_D:
        handle_ctrl_d(self_p);
        break;

    case CTRL_K:
        handle_ctrl_k(self_p);
        break;

    case CTRL_T:
        handle_ctrl_t(self_p);
        break;

    case ALT:
        return (handle_alt(self_p));

    default:
        handle_other(self_p, ch);
        break;
    }

    /* Show the new line to the user and execute it if enter was
       pressed. */
    show_line(self_p);

    if (self_p->newline_received) {
        return (execute_line(self_p));
    }

    return (-1);
}

/**
 * Read the next command. Return -EAGAIN if more input is
 * needed. Returns the command length once a command has been
 * received.
 */
static int read_command(struct async_shell_t *self_p)
{
    int res;

    res = -1;

    while (res == -1) {
        switch (self_p->command_reader_state) {

        case async_shell_command_reader_state_init_t:
            res = read_command_init(self_p);
            break;

        case async_shell_command_reader_state_alt_t:
            res = read_command_alt(self_p);
            break;

        case async_shell_command_reader_state_alt_o_t:
            res = read_command_alt_o(self_p);
            break;

        case async_shell_command_reader_state_alt_bracket_t:
            res = read_command_alt_bracket(self_p);
            break;

        default:
            res = read_command_any(self_p);
            break;
        }
    }

    return (res);
}

static void on_opened(struct async_shell_t *self_p, int res)
{
    if (res == 0) {
        output(self_p, "\nWelcome to the async shell!\n\n");
        print_prompt(self_p);
    } else {
        printf("Failed to open the shell channel.\n");
    }
}

static void on_closed(struct async_shell_t *self_p)
{
    (void)self_p;

    printf("Shell channel closed!\n");
}

static void on_input(struct async_shell_t *self_p)
{
    int res;
    char *stripped_line_p;

    res = read_command(self_p);

    if (res == -EAGAIN) {
        return;
    }

    if (res > 0) {
        stripped_line_p = hf_strip(line_get_buf(&self_p->line), NULL);

        if (!is_comment(stripped_line_p)) {
            res = execute_command(self_p, stripped_line_p);

            if (res == 0) {
                output(self_p, "OK\n");
            } else {
                output_sds(self_p, "ERROR(", res, ")\n");
            }
        }
    }

    print_prompt(self_p);
    self_p->command_reader_state = async_shell_command_reader_state_init_t;
}

void async_shell_init(struct async_shell_t *self_p,
                      struct async_channel_t *channel_p,
                      struct async_t *async_p)
{
    self_p->commands_p = NULL;
    self_p->async_p = async_p;
    async_channel_set_on(channel_p,
                         (async_channel_opened_t)on_opened,
                         (async_func_t)on_closed,
                         (async_func_t)on_input,
                         self_p);
    self_p->channel_p = channel_p;
    history_init(self_p);
    self_p->command_reader_state = async_shell_command_reader_state_init_t;

    async_shell_command_init(&self_p->commands.help,
                             "help",
                             "Print this help.",
                             command_help);
    async_shell_register_command(self_p, &self_p->commands.help);
    async_shell_command_init(&self_p->commands.history,
                             "history",
                             "List command history.",
                             command_history);
    async_shell_register_command(self_p, &self_p->commands.history);
}

void async_shell_start(struct async_shell_t *self_p)
{
    async_channel_open(self_p->channel_p);
}

void async_shell_stop(struct async_shell_t *self_p)
{
    async_channel_close(self_p->channel_p);
}

void async_shell_command_init(struct async_shell_command_t *self_p,
                              const char *name_p,
                              const char *description_p,
                              async_shell_command_t callback)
{
    self_p->name_p = name_p;
    self_p->description_p = description_p;
    self_p->callback = callback;
}

void async_shell_register_command(struct async_shell_t *self_p,
                                  struct async_shell_command_t *command_p)
{
    struct async_shell_command_t *curr_p;
    struct async_shell_command_t *prev_p;

    /* Insert in alphabetical order. */
    prev_p = NULL;
    curr_p = self_p->commands_p;

    while (curr_p != NULL) {
        if (strcmp(command_p->name_p, curr_p->name_p) < 0) {
            break;
        }

        prev_p = curr_p;
        curr_p = curr_p->next_p;
    }

    /* Insert into the list. */
    command_p->next_p = curr_p;

    if (prev_p == NULL) {
        self_p->commands_p = command_p;
    } else {
        prev_p->next_p = command_p;
    }
}
