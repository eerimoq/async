#include "nala.h"
#include "nala_mocks.h"
#include "async.h"

TEST(process_empty)
{
    struct async_t async;

    async_init(&async);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_NO_ACTION);
    async_destroy(&async);
}

static void increment(int *arg_p)
{
    (*arg_p)++;
}

TEST(call_once)
{
    struct async_t async;
    int arg;

    async_init(&async);
    arg = 0;
    ASSERT_EQ(async_call(&async, (async_func_t)increment, &arg), 0);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_NO_ACTION);
    ASSERT_EQ(arg, 1);
    async_destroy(&async);
}

TEST(call_twice)
{
    struct async_t async;
    int arg;

    async_init(&async);
    arg = 0;
    ASSERT_EQ(async_call(&async, (async_func_t)increment, &arg), 0);
    ASSERT_EQ(async_call(&async, (async_func_t)increment, &arg), 0);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_NO_ACTION);
    ASSERT_EQ(arg, 2);
    async_destroy(&async);
}

TEST(call_queue_full)
{
    struct async_t async;
    int arg;
    int i;

    async_init(&async);
    arg = 0;

    for (i = 0; i < 32; i++) {
        ASSERT_EQ(async_call(&async, (async_func_t)increment, &arg), 0);
    }

    ASSERT_EQ(async_call(&async, (async_func_t)increment, &arg),
              -ASYNC_ERROR_QUEUE_FULL);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_NO_ACTION);
    ASSERT_EQ(arg, 32);
    async_destroy(&async);
}

static int log_print_object;

static void log_stdout(void *log_object_p,
                       int level,
                       const char *fmt_p,
                       ...)
{
    va_list vlist;

    ASSERT_EQ(log_object_p, &log_print_object);
    ASSERT_EQ(level, ASYNC_LOG_INFO);

    va_start(vlist, fmt_p);
    vprintf(fmt_p, vlist);
    printf("\n");
    va_end(vlist);
}

TEST(log_print)
{
    struct async_t async;

    async_init(&async);

    /* First log using default functions. No output. */
    CAPTURE_OUTPUT(output, errput) {
        async.log_object.print(&log_print_object, ASYNC_LOG_INFO, "Hello!");
    }

    ASSERT_EQ(output, "");
    ASSERT_EQ(errput, "");

    /* Now set log callbacks to print to stdout. */
    async_set_log_object_callbacks(&async, log_stdout, NULL);

    CAPTURE_OUTPUT(output2, errput2) {
        async.log_object.print(&log_print_object, ASYNC_LOG_INFO, "Hello!");
    }

    ASSERT_EQ(output2, "Hello!\n");
    ASSERT_EQ(errput2, "");

    /* Set back to default. No output once again. */
    async_set_log_object_callbacks(&async, NULL, NULL);

    CAPTURE_OUTPUT(output3, errput3) {
        async.log_object.print(&log_print_object, ASYNC_LOG_INFO, "Hello!");
    }

    ASSERT_EQ(output3, "");
    ASSERT_EQ(errput3, "");
}

static int log_is_enabled_for_object;

static bool log_is_enabled_for_only_info(void *log_object_p,
                                         int level)
{
    ASSERT_EQ(log_object_p, &log_is_enabled_for_object);

    return (level == ASYNC_LOG_INFO);
}

TEST(log_is_enabled_for)
{
    struct async_t async;

    async_init(&async);

    /* First check using default functions. Not enabled. */
    ASSERT(!async.log_object.is_enabled_for(&log_is_enabled_for_object,
                                            ASYNC_LOG_DEBUG));
    ASSERT(!async.log_object.is_enabled_for(&log_is_enabled_for_object,
                                            ASYNC_LOG_INFO));

    /* Now set log callbacks to enable logging. */
    async_set_log_object_callbacks(&async, NULL, log_is_enabled_for_only_info);

    ASSERT(!async.log_object.is_enabled_for(&log_is_enabled_for_object,
                                            ASYNC_LOG_DEBUG));
    ASSERT(async.log_object.is_enabled_for(&log_is_enabled_for_object,
                                           ASYNC_LOG_INFO));

    /* Set back to default. Not enabled once again. */
    async_set_log_object_callbacks(&async, NULL, NULL);

    ASSERT(!async.log_object.is_enabled_for(&log_is_enabled_for_object,
                                            ASYNC_LOG_DEBUG));
    ASSERT(!async.log_object.is_enabled_for(&log_is_enabled_for_object,
                                            ASYNC_LOG_INFO));
}
