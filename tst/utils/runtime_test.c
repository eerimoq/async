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
#include "nala.h"
#include "async/core/runtime.h"
#include "async/runtimes/linux.h"
#include "runtime_test.h"

static struct async_runtime_t runtime = {
    .set_async = runtime_test_set_async,
    .run_forever = runtime_test_run_forever,
    .tcp_client = {
        .init = runtime_test_tcp_client_init,
        .connect = runtime_test_tcp_client_connect,
        .disconnect = runtime_test_tcp_client_disconnect,
        .write = runtime_test_tcp_client_write,
        .read = runtime_test_tcp_client_read
    },
    .tcp_server = {
        .init = runtime_test_tcp_server_init,
        .start = runtime_test_tcp_server_start,
        .stop = runtime_test_tcp_server_stop,
        .client = {
            .write = runtime_test_tcp_server_client_write,
            .read = runtime_test_tcp_server_client_read,
            .disconnect = runtime_test_tcp_server_client_disconnect
        }
    }
};

struct async_runtime_t *runtime_test_create()
{
    return (&runtime);
}
