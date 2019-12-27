#include "nala.h"
#include "nala_mocks.h"
#include "async.h"

TEST(process_empty)
{
    struct async_t async;

    async_init(&async);
    ASSERT_EQ(async_process(&async, 0), -1);
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
    ASSERT_EQ(async_process(&async, 0), -1);
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
    ASSERT_EQ(async_process(&async, 0), -1);
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
    ASSERT_EQ(async_process(&async, 0), -1);
    ASSERT_EQ(arg, 32);
    async_destroy(&async);
}
