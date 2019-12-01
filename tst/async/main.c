#include "nala.h"
#include "nala_mocks.h"
#include "async.h"

TEST(test_process_empty)
{
    struct async_t async;

    async_init(&async, 100);
    async_run_until_complete(&async);
    async_destroy(&async);
}

static void increment(int *arg_p)
{
    (*arg_p)++;
}

TEST(test_call_once)
{
    struct async_t async;
    int arg;

    async_init(&async, 100);
    arg = 0;
    ASSERT_EQ(async_call(&async, (async_func_t)increment, &arg), 0);
    async_run_until_complete(&async);
    ASSERT_EQ(arg, 1);
    async_destroy(&async);
}

TEST(test_call_twice)
{
    struct async_t async;
    int arg;

    async_init(&async, 100);
    arg = 0;
    ASSERT_EQ(async_call(&async, (async_func_t)increment, &arg), 0);
    ASSERT_EQ(async_call(&async, (async_func_t)increment, &arg), 0);
    async_run_until_complete(&async);
    ASSERT_EQ(arg, 2);
    async_destroy(&async);
}

TEST(test_call_queue_full)
{
    struct async_t async;
    int arg;
    int i;

    async_init(&async, 100);
    arg = 0;

    for (i = 0; i < 32; i++) {
        ASSERT_EQ(async_call(&async, (async_func_t)increment, &arg), 0);
    }

    ASSERT_EQ(async_call(&async, (async_func_t)increment, &arg),
              -ASYNC_ERROR_QUEUE_FULL);
    async_run_until_complete(&async);
    ASSERT_EQ(arg, 32);
    async_destroy(&async);
}

TEST(test_single_shot_timer)
{
    struct async_t async;
    struct async_timer_t timer;
    int arg;

    async_init(&async, 100);
    async_timer_init(&timer,
                     (async_func_t)increment,
                     &arg,
                     0,
                     &async);
    arg = 0;
    ASSERT_EQ(async_timer_is_stopped(&timer), false);
    async_timer_start(&timer, 0);
    ASSERT_EQ(async_timer_is_stopped(&timer), false);
    async_tick(&async);
    async_tick(&async);
    async_run_until_complete(&async);
    ASSERT_EQ(async_timer_is_stopped(&timer), false);
    ASSERT_EQ(arg, 1);
    async_destroy(&async);
}

TEST(test_single_shot_timer_stop_running)
{
    struct async_t async;
    struct async_timer_t timer;
    int arg;

    async_init(&async, 100);
    async_timer_init(&timer,
                     (async_func_t)increment,
                     &arg,
                     0,
                     &async);
    arg = 0;
    async_timer_start(&timer, 0);
    ASSERT_EQ(async_timer_is_stopped(&timer), false);
    async_timer_stop(&timer);
    ASSERT_EQ(async_timer_is_stopped(&timer), true);
    async_destroy(&async);
}

TEST(test_single_shot_timer_stop_expired)
{
    struct async_t async;
    struct async_timer_t timer;
    int arg;

    async_init(&async, 100);
    async_timer_init(&timer,
                     (async_func_t)increment,
                     &arg,
                     0,
                     &async);
    arg = 0;
    async_timer_start(&timer, 0);
    ASSERT_EQ(async_timer_is_stopped(&timer), false);
    async_tick(&async);
    async_tick(&async);
    async_run_until_complete(&async);
    ASSERT_EQ(async_timer_is_stopped(&timer), false);
    ASSERT_EQ(arg, 1);
    async_timer_stop(&timer);
    ASSERT_EQ(async_timer_is_stopped(&timer), true);
    async_destroy(&async);
}
