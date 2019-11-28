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

/* Needed by ftw. */
#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE

#include <dbg.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <ctype.h>
#include <unistd.h>
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
#define CTRL_R                                    18
#define CTRL_G                                     7
#define ALT                                       27

/* static void history_init(struct async_shell_t *self_p); */
static void show_line(struct async_shell_t *self_p);

static void output(struct async_shell_t *self_p, const char *string_p)
{
    async_channel_write(self_p->channel_p, string_p, strlen(string_p));
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

static int compare_bsearch(const void *key_p, const void *elem_p)
{
    const char *name_p;
    const char *elem_name_p;

    name_p = (const char *)key_p;
    elem_name_p = ((struct async_shell_command_t *)elem_p)->name_p;

    return (strcmp(name_p, elem_name_p));
}

static int compare_qsort(const void *lelem_p, const void *relem_p)
{
    const char *lname_p;
    const char *rname_p;
    int res;

    lname_p = ((struct async_shell_command_t *)lelem_p)->name_p;
    rname_p = ((struct async_shell_command_t *)relem_p)->name_p;

    res = strcmp(lname_p, rname_p);

    if (res == 0) {
        printf("%s: shell commands must be unique", lname_p);
        exit(1);
    }

    return (res);
}

static struct async_shell_command_t *find_command(struct async_shell_t *self_p,
                                                  const char *name_p)
{
    return (bsearch(name_p,
                    self_p->commands_p,
                    self_p->number_of_commands,
                    sizeof(*self_p->commands_p),
                    compare_bsearch));
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

/* static void line_insert_string(struct async_shell_line_t *self_p, */
/*                                char *str_p) */
/* { */
/*     while (*str_p != '\0') { */
/*         if (!line_insert(self_p, *str_p)) { */
/*             break; */
/*         } */

/*         str_p++; */
/*     } */
/* } */

/* static void line_delete(struct async_shell_line_t *self_p) */
/* { */
/*     /\* End of buffer? *\/ */
/*     if (self_p->cursor == self_p->length) { */
/*         return; */
/*     } */

/*     /\* Move the string, including the NULL termination, one step to */
/*        the left to overwrite the deleted character. *\/ */
/*     memmove(&self_p->buf[self_p->cursor], */
/*             &self_p->buf[self_p->cursor + 1], */
/*             self_p->length - self_p->cursor); */
/*     self_p->length--; */
/* } */

/* static int line_peek(struct async_shell_line_t *self_p) */
/* { */
/*     return (self_p->buf[self_p->cursor]); */
/* } */

/* static void line_truncate(struct async_shell_line_t *self_p) */
/* { */
/*     self_p->length = self_p->cursor; */
/*     self_p->buf[self_p->length] = '\0'; */
/* } */

/* static bool line_is_empty(struct async_shell_line_t *self_p) */
/* { */
/*     return (self_p->length == 0); */
/* } */

static char *line_get_buf(struct async_shell_line_t *self_p)
{
    return (self_p->buf);
}

static int line_get_length(struct async_shell_line_t *self_p)
{
    return (self_p->length);
}

/* static bool line_seek(struct async_shell_line_t *self_p, int pos) */
/* { */
/*     if (pos < 0) { */
/*         if ((self_p->cursor + pos) < 0) { */
/*             return (false); */
/*         } */
/*     } else { */
/*         if ((self_p->cursor + pos) > self_p->length) { */
/*             return (false); */
/*         } */
/*     } */

/*     self_p->cursor += pos; */

/*     return (true); */
/* } */

static int line_get_cursor(struct async_shell_line_t *self_p)
{
    return (self_p->cursor);
}

/* static void line_seek_begin(struct async_shell_line_t *self_p) */
/* { */
/*     self_p->cursor = 0; */
/* } */

/* static void line_seek_end(struct async_shell_line_t *self_p) */
/* { */
/*     self_p->cursor = self_p->length; */
/* } */

static int command_help(struct async_shell_t *self_p,
                        int argc,
                        const char *argv[])
{
    (void)argc;
    (void)argv;

    int i;
    char buf[128];

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
           "       Ctrl+R   Recall the last command including the specified "
           "character(s)\n"
           "                searches the command history as you type.\n"
           "       Ctrl+G   Escape from history searching mode.\n"
           "\n"
           "Commands\n"
           "\n");

    for (i = 0; i < self_p->number_of_commands; i++) {
        snprintf(&buf[0],
                 sizeof(buf),
                 "%13s   %s\n",
                 self_p->commands_p[i].name_p,
                 self_p->commands_p[i].description_p);
        buf[sizeof(buf) - 1] = '\0';
        output(self_p, &buf[0]);
    }

    return (0);
}

#if 0

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
        printf("%d: %s\n", i, current_p->buf);
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

static char *history_reverse_search(struct async_shell_t *self_p,
                                    const char *pattern_p)
{
    struct async_shell_history_elem_t *elem_p;

    elem_p = self_p->history.tail_p;

    while (elem_p != NULL) {
        if (strstr(elem_p->buf, pattern_p) != NULL) {
            return (elem_p->buf);
        }

        elem_p = elem_p->prev_p;
    }

    return (NULL);
}

static void auto_complete_command(struct async_shell_t *self_p)
{
    char next_char;
    bool mismatch;
    int size;
    int i;
    int first_match;
    bool completion_happend;
    char *line_p;

    line_p = line_get_buf(&self_p->line);
    size = line_get_length(&self_p->line);

    /* Find the first command matching given line. */
    first_match = -1;

    for (i = 0; i < self_p->number_of_commands; i++) {
        if (strncmp(self_p->commands_p[i].name_p, line_p, size) == 0) {
            first_match = i;
            break;
        }
    }

    /* No command matching the line. */
    if (first_match == -1) {
        return;
    }

    /* Auto-complete given line. Compare the next character each
       iteration. */
    completion_happend = false;

    while (true) {
        mismatch = false;
        next_char = self_p->commands_p[first_match].name_p[size];

        /* It's a match if all commands matching line has the same
           next character. */
        i = first_match;

        while ((i < self_p->number_of_commands)
               && (strncmp(&self_p->commands_p[i].name_p[0],
                           line_p,
                           size) == 0)) {
            if (self_p->commands_p[i].name_p[size] != next_char) {
                mismatch = true;
                break;
            }

            i++;
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
        putchar('\n');
        i = first_match;

        while ((i < self_p->number_of_commands)
               && (strncmp(self_p->commands_p[i].name_p,
                           line_p,
                           size) == 0)) {
            printf("%s\n", self_p->commands_p[i].name_p);
            i++;
        }

        print_prompt(self_p);
        printf("%s", line_get_buf(&self_p->line));
    }
}

static void handle_tab(struct async_shell_t *self_p)
{
    auto_complete_command(self_p);
}

#endif

static void handle_carrige_return(struct async_shell_t *self_p)
{
    self_p->carriage_return_received = true;
}

static void handle_newline(struct async_shell_t *self_p)
{
    self_p->newline_received = true;
}

#if 0

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

static void restore_previous_line(struct async_shell_t *self_p,
                                  struct async_shell_line_t *pattern_p)
{
    int cursor;
    int length;

    printf("\x1b[%dD\x1b[K%s",
           17 + line_get_length(pattern_p),
           line_get_buf(&self_p->prev_line));
    fflush(stdout);

    cursor = line_get_cursor(&self_p->prev_line);
    length = line_get_length(&self_p->prev_line);

    if (cursor != length) {
        printf("\x1b[%dD", length - cursor);
        fflush(stdout);
    }
}

/**
 * Ctrl+R Recall the last command including the specified character(s)
 * searches the command history as you type.
 *
 * The original line buffer is printed and cursor reset, then the
 * selected command is copied into the line buffer. The output of the
 * new command occurs in the main command loop.
 */
static void handle_ctrl_r(struct async_shell_t *self_p)
{
    int ch;
    char *buf_p;

    line_init(&self_p->history.pattern);
    line_init(&self_p->history.match);

    if (!line_is_empty(&self_p->line)) {
        printf("\x1b[%dD", line_get_length(&self_p->line));
    }

    printf("\x1b[K(history-search)`': \x1b[3D");

    while (true) {
        fflush(stdout);
        ch = xgetc(self_p);

        switch (ch) {

        case DELETE:
        case BACKSPACE:
            if (!line_is_empty(&self_p->history.pattern)) {
                printf("\x1b[1D\x1b[K': ");
                line_seek(&self_p->history.pattern, -1);
                line_delete(&self_p->history.pattern);
                buf_p = history_reverse_search(
                    line_get_buf(&self_p->history.pattern));
                line_init(&self_p->history.match);

                if (buf_p != NULL) {
                    line_insert_string(&self_p->history.match, buf_p);
                }

                printf("%s\x1b[%dD",
                       line_get_buf(&self_p->history.match),
                       line_get_length(&self_p->history.match) + 3);
            }

            break;

        case CARRIAGE_RETURN:
            self_p->carriage_return_received = true;
            break;

        case CTRL_G:
            restore_previous_line(&self_p->history.pattern);
            return;

        default:
            if (isprint(ch)) {
                if (line_insert(&self_p->history.pattern, ch)) {
                    printf("\x1b[K%c': ", ch);
                    buf_p = history_reverse_search(
                        line_get_buf(&self_p->history.pattern));
                    line_init(&self_p->history.match);

                    if (buf_p != NULL) {
                        line_insert_string(&self_p->history.match, buf_p);
                    }

                    printf("%s\x1b[%dD",
                           line_get_buf(&self_p->history.match),
                           line_get_length(&self_p->history.match) + 3);
                }
            } else {
                restore_previous_line(&self_p->history.pattern);

                /* Copy the match to current line. */
                self_p->line = self_p->history.match;

                if (ch == NEWLINE) {
                    self_p->newline_received = true;
                } else {
                    if (ch == ALT) {
                        ch = xgetc(self_p);

                        if (ch != 'd') {
                            (void)xgetc();
                        }
                    }
                }

                return;
            }
        }
    }
}

/**
 * ALT.
 */
static void handle_alt(struct async_shell_t *self_p)
{
    int ch;
    char *buf_p;

    ch = xgetc();

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

        break;

    case 'O':
        ch = xgetc();

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

        break;

    case '[':
        ch = xgetc();

        switch (ch) {

        case 'A':
        case 'B':
            if (ch == 'A') {
                /* UP Previous command. */
                buf_p = history_get_previous_command();
            } else {
                /* DOWN Next command. */
                buf_p = history_get_next_command();
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

        break;

    default:
        break;
    }
}

#endif

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
                printf("%s", &line_get_buf(&self_p->line)[cursor]);
            } else {
                /* Move the cursor to the end of the old line. */
                for (i = cursor; i < length; i++) {
                    printf(" ");
                }

                /* Backspace. */
                for (i = new_length; i < length; i++) {
                    printf("\x08 \x08");
                }
            }
        } else {
            if (cursor > 0) {
                printf("\x1b[%dD", cursor);
            }

            printf("\x1b[K%s", line_get_buf(&self_p->line));

            if (new_cursor < new_length) {
                printf("\x1b[%dD", new_length - new_cursor);
            }
        }
    } else if (cursor < new_cursor) {
        printf("\x1b[%dC", new_cursor - cursor);
    } else if (new_cursor < cursor) {
        printf("\x1b[%dD", cursor - new_cursor);
    }

    fflush(stdout);
}

/**
 * Execute the current line.
 */
static int execute_line(struct async_shell_t *self_p)
{
    if (self_p->carriage_return_received) {
        printf("\r");
    }

    printf("\n");
    fflush(stdout);
    
    /* /\* Append the command to the history. *\/ */
    /* if (!line_is_empty(&self_p->line)) { */
    /*     history_append(self_p, line_get_buf(&self_p->line)); */
    /* } */

    /* history_reset_current(self_p); */

    return (line_get_length(&self_p->line));
}

static int read_command_init(struct async_shell_t *self_p)
{
    /* dbg(""); */
    line_init(&self_p->line);
    self_p->carriage_return_received = false;
    self_p->newline_received = false;
    self_p->command_reader_state = async_shell_command_reader_state_read_t;

    return (-1);
}

static int read_command_any(struct async_shell_t *self_p)
{
    int ch;

    /* dbg(""); */

    ch = xgetc(self_p);

    if (ch == -EAGAIN) {
        return (ch);
    }

    /* Save current line. */
    self_p->prev_line = self_p->line;

    switch (ch) {

    /* case TAB: */
    /*     handle_tab(); */
    /*     break; */

    case CARRIAGE_RETURN:
        handle_carrige_return(self_p);
        break;

    case NEWLINE:
        handle_newline(self_p);
        break;

    /* case DELETE: */
    /* case BACKSPACE: */
    /*     handle_backspace(); */
    /*     break; */

    /* case CTRL_A: */
    /*     handle_ctrl_a(); */
    /*     break; */

    /* case CTRL_E: */
    /*     handle_ctrl_e(); */
    /*     break; */

    /* case CTRL_D: */
    /*     handle_ctrl_d(); */
    /*     break; */

    /* case CTRL_K: */
    /*     handle_ctrl_k(); */
    /*     break; */

    /* case CTRL_T: */
    /*     handle_ctrl_t(); */
    /*     break; */

    /* case CTRL_R: */
    /*     handle_ctrl_r(); */
    /*     break; */

    /* case ALT: */
    /*     handle_alt(); */
    /*     break; */

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

        /* case async_shell_command_reader_state_ctrl_r_t: */
        /*     res = read_command_ctrl_r(self_p); */
        /*     break; */

        /* case command_reader_state_alt_t: */
        /*     res = read_command_alt(self_p); */
        /*     break; */

        default:
            res = read_command_any(self_p);
            break;
        }
    }

    return (res);
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
                printf("OK\n");
            } else {
                printf("ERROR(%d)\n", res);
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
    self_p->number_of_commands = 0;
    self_p->commands_p = malloc(1);
    self_p->opened = false;
    self_p->async_p = async_p;
    async_channel_set_on(channel_p,
                         NULL,
                         NULL,
                         (async_func_t)on_input,
                         self_p);
    self_p->channel_p = channel_p;
    /* history_init(self_p); */
    self_p->command_reader_state = async_shell_command_reader_state_init_t;

    async_shell_register_command(self_p,
                                 "help",
                                 "Print this help.",
                                 command_help);
    /* async_shell_register_command(self_p, */
    /*                              "history", */
    /*                              "List command history.", */
    /*                              command_history); */
}

void async_shell_start(struct async_shell_t *self_p)
{
    qsort(self_p->commands_p,
          self_p->number_of_commands,
          sizeof(*self_p->commands_p),
          compare_qsort);
    async_channel_open(self_p->channel_p);
}

void async_shell_stop(struct async_shell_t *self_p)
{
    async_channel_close(self_p->channel_p);
}

void async_shell_register_command(struct async_shell_t *self_p,
                                  const char *name_p,
                                  const char *description_p,
                                  async_shell_command_t callback)
{
    struct async_shell_command_t *command_p;

    self_p->number_of_commands++;
    self_p->commands_p = realloc(
        self_p->commands_p,
        sizeof(*self_p->commands_p) * self_p->number_of_commands);
    command_p = &self_p->commands_p[self_p->number_of_commands - 1];
    command_p->name_p = name_p;
    command_p->description_p = description_p;
    command_p->callback = callback;
}
