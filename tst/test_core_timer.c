#include "nala.h"
#include "nala_mocks.h"
#include "async.h"
#include "utils.h"

struct counter_t {
    struct async_timer_t timer;
    int value;
};

static void on_timeout(struct async_timer_t *timer_p)
{
    struct counter_t *self_p;

    self_p = async_container_of(timer_p, typeof(*self_p), timer);
    self_p->value++;
}

TEST(single_shot_timer)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     on_timeout,
                     0,
                     0,
                     &async);
    counter.value = 0;
    mock_prepare_time_ms(0);
    async_timer_start(&counter.timer);
    mock_prepare_process(0, -1);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_NO_ACTION);
    ASSERT_EQ(counter.value, 1);
    async_destroy(&async);
}

/**
 * Starting a timer before calling process resultet in process
 * returning -ASYNC_ERROR_TIMER_NO_ACTION indefinitely.
 */
TEST(start_before_process_called)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     on_timeout,
                     1000,
                     0,
                     &async);
    counter.value = 0;
    mock_prepare_time_ms(0);
    async_timer_start(&counter.timer);
    mock_prepare_process(0, 0);
    ASSERT_EQ(async_process(&async), 1000);
    ASSERT_EQ(counter.value, 0);
    mock_prepare_process(500, -1);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_NO_ACTION);
    ASSERT_EQ(counter.value, 0);
    mock_prepare_process(1000, -1);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_LAST_STOPPED);
    ASSERT_EQ(counter.value, 1);
    async_destroy(&async);
}

TEST(single_shot_timer_stop_running)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     on_timeout,
                     0,
                     0,
                     &async);
    counter.value = 0;
    mock_prepare_time_ms(0);
    async_timer_start(&counter.timer);
    async_timer_stop(&counter.timer);
    mock_prepare_process(0, -1);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_NO_ACTION);
    async_destroy(&async);
}

static void stop_timer(struct async_timer_t *timer_p)
{
    async_timer_stop(timer_p);
}

TEST(single_shot_timer_stop_expired_before_handled)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     on_timeout,
                     0,
                     0,
                     &async);
    counter.value = 0;
    mock_prepare_time_ms(0);
    async_timer_start(&counter.timer);
    async_call(&async, (async_func_t)stop_timer, &counter.timer);
    mock_prepare_process(0, -1);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_NO_ACTION);
    ASSERT_EQ(counter.value, 0);
    async_destroy(&async);
}

TEST(timer_get_and_set_initial_and_repeat)
{
    struct async_t async;
    struct async_timer_t timer;

    async_init(&async);
    async_timer_init(&timer,
                     on_timeout,
                     1,
                     2,
                     &async);
    ASSERT_EQ(async_timer_get_initial(&timer), 1u);
    ASSERT_EQ(async_timer_get_repeat(&timer), 2u);
    async_timer_set_initial(&timer, 3);
    async_timer_set_repeat(&timer, 4);
    ASSERT_EQ(async_timer_get_initial(&timer), 3u);
    ASSERT_EQ(async_timer_get_repeat(&timer), 4u);
}

TEST(initial_and_repeat)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     on_timeout,
                     300,
                     100,
                     &async);
    counter.value = 0;
    mock_prepare_time_ms(0);
    async_timer_start(&counter.timer);

    /* Initial timeout. */
    mock_prepare_process(300, 301);
    ASSERT_EQ(async_process(&async), 99);
    ASSERT_EQ(counter.value, 1);

    /* Repeated timeout 1. */
    mock_prepare_process(400, 400);
    ASSERT_EQ(async_process(&async), 100);
    ASSERT_EQ(counter.value, 2);

    /* Repeated timeout 2. */
    mock_prepare_process(500, 500);
    ASSERT_EQ(async_process(&async), 100);
    ASSERT_EQ(counter.value, 3);

    /* Repeated timeout 3. */
    mock_prepare_process(600, 600);
    ASSERT_EQ(async_process(&async), 100);
    ASSERT_EQ(counter.value, 4);

    /* Change repeat. Takes effect after current expiry. */
    async_timer_set_repeat(&counter.timer, 200);

    /* Repeated timeout 4. */
    mock_prepare_process(700, 700);
    ASSERT_EQ(async_process(&async), 200);
    ASSERT_EQ(counter.value, 5);

    /* Repeated timeout 5. */
    mock_prepare_process(900, 900);
    ASSERT_EQ(async_process(&async), 200);
    ASSERT_EQ(counter.value, 6);

    /* Repeated timeout 6. */
    mock_prepare_process(1100, 1100);
    ASSERT_EQ(async_process(&async), 200);
    ASSERT_EQ(counter.value, 7);

    /* Stop the timer, set initial and repeat and start it again. */
    async_timer_stop(&counter.timer);
    async_timer_set_initial(&counter.timer, 0);
    async_timer_set_repeat(&counter.timer, 100);
    mock_prepare_time_ms(1100);
    async_timer_start(&counter.timer);

    /* Inital timeout. */
    mock_prepare_process(1100, 1100);
    ASSERT_EQ(async_process(&async), 100);
    ASSERT_EQ(counter.value, 8);

    /* Repeated timeout 1. */
    mock_prepare_process(1200, 1200);
    ASSERT_EQ(async_process(&async), 100);
    ASSERT_EQ(counter.value, 9);

    /* Repeated timeout 2. */
    mock_prepare_process(1300, 1300);
    ASSERT_EQ(async_process(&async), 100);
    ASSERT_EQ(counter.value, 10);

    async_destroy(&async);
}

static void restart_timer(struct async_timer_t *timer_p)
{
    async_timer_stop(timer_p);
    mock_prepare_time_ms(0);
    async_timer_start(timer_p);
}

TEST(restart_with_outstanding_timeout)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     on_timeout,
                     0,
                     0,
                     &async);
    counter.value = 0;
    mock_prepare_time_ms(0);
    async_timer_start(&counter.timer);
    async_call(&async, (async_func_t)restart_timer, &counter.timer);
    mock_prepare_process(0, 0);
    ASSERT_EQ(async_process(&async), 0);
    ASSERT_EQ(counter.value, 0);
    mock_prepare_process(0, -1);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_LAST_STOPPED);
    ASSERT_EQ(counter.value, 1);
    async_destroy(&async);
}

TEST(restart_with_outstanding_timeouts)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     on_timeout,
                     0,
                     100,
                     &async);
    counter.value = 0;

    /* Make sure it does not expire as its stopped.  */
    mock_prepare_time_ms(0);
    async_timer_start(&counter.timer);
    async_timer_stop(&counter.timer);
    mock_prepare_process(500, -1);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_NO_ACTION);

    /* Start again and count 3 timeouts. */
    mock_prepare_time_ms(500);
    async_timer_start(&counter.timer);

    mock_prepare_process(700, 700);
    ASSERT_EQ(async_process(&async), 100);
    mock_prepare_process(800, 800);
    ASSERT_EQ(async_process(&async), 100);
    mock_prepare_process(900, 900);
    ASSERT_EQ(async_process(&async), 100);

    ASSERT_EQ(counter.value, 3);

    async_destroy(&async);
}

TEST(multiple_timers)
{
    int timeouts[10] = {
        50, 0, 100, 75, 50, 50, 100, 90, 10, 0
    };
    struct counter_t counters[10];
    int i;
    struct async_t async;

    async_init(&async);

    for (i = 0; i < 10; i++) {
        counters[i].value = 0;
        async_timer_init(&counters[i].timer,
                         on_timeout,
                         timeouts[i],
                         0,
                         &async);
        mock_prepare_time_ms(0);
        async_timer_start(&counters[i].timer);
    }

    /* 0 ms timers expires. */
    mock_prepare_process(5, 5);
    ASSERT_EQ(async_process(&async), 5);
    ASSERT_EQ(counters[0].value, 0)
    ASSERT_EQ(counters[1].value, 1)
    ASSERT_EQ(counters[2].value, 0)
    ASSERT_EQ(counters[3].value, 0)
    ASSERT_EQ(counters[4].value, 0)
    ASSERT_EQ(counters[5].value, 0)
    ASSERT_EQ(counters[6].value, 0)
    ASSERT_EQ(counters[7].value, 0)
    ASSERT_EQ(counters[8].value, 0)
    ASSERT_EQ(counters[9].value, 1)

    /* 10 ms timer expire. */
    mock_prepare_process(10, 10);
    ASSERT_EQ(async_process(&async), 40);
    ASSERT_EQ(counters[0].value, 0)
    ASSERT_EQ(counters[1].value, 1)
    ASSERT_EQ(counters[2].value, 0)
    ASSERT_EQ(counters[3].value, 0)
    ASSERT_EQ(counters[4].value, 0)
    ASSERT_EQ(counters[5].value, 0)
    ASSERT_EQ(counters[6].value, 0)
    ASSERT_EQ(counters[7].value, 0)
    ASSERT_EQ(counters[8].value, 1)
    ASSERT_EQ(counters[9].value, 1)

    /* 50 ms timers expires. */
    mock_prepare_process(50, 50);
    ASSERT_EQ(async_process(&async), 25);
    ASSERT_EQ(counters[0].value, 1)
    ASSERT_EQ(counters[1].value, 1)
    ASSERT_EQ(counters[2].value, 0)
    ASSERT_EQ(counters[3].value, 0)
    ASSERT_EQ(counters[4].value, 1)
    ASSERT_EQ(counters[5].value, 1)
    ASSERT_EQ(counters[6].value, 0)
    ASSERT_EQ(counters[7].value, 0)
    ASSERT_EQ(counters[8].value, 1)
    ASSERT_EQ(counters[9].value, 1)

    /* 75 ms timer expire. */
    mock_prepare_process(75, 75);
    ASSERT_EQ(async_process(&async), 15);
    ASSERT_EQ(counters[0].value, 1)
    ASSERT_EQ(counters[1].value, 1)
    ASSERT_EQ(counters[2].value, 0)
    ASSERT_EQ(counters[3].value, 1)
    ASSERT_EQ(counters[4].value, 1)
    ASSERT_EQ(counters[5].value, 1)
    ASSERT_EQ(counters[6].value, 0)
    ASSERT_EQ(counters[7].value, 0)
    ASSERT_EQ(counters[8].value, 1)
    ASSERT_EQ(counters[9].value, 1)

    /* 90 ms timer expire. */
    mock_prepare_process(90, 90);
    ASSERT_EQ(async_process(&async), 10);
    ASSERT_EQ(counters[0].value, 1)
    ASSERT_EQ(counters[1].value, 1)
    ASSERT_EQ(counters[2].value, 0)
    ASSERT_EQ(counters[3].value, 1)
    ASSERT_EQ(counters[4].value, 1)
    ASSERT_EQ(counters[5].value, 1)
    ASSERT_EQ(counters[6].value, 0)
    ASSERT_EQ(counters[7].value, 1)
    ASSERT_EQ(counters[8].value, 1)
    ASSERT_EQ(counters[9].value, 1)

    /* 100 ms timers expires. */
    mock_prepare_process(100, -1);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_LAST_STOPPED);
    ASSERT_EQ(counters[0].value, 1)
    ASSERT_EQ(counters[1].value, 1)
    ASSERT_EQ(counters[2].value, 1)
    ASSERT_EQ(counters[3].value, 1)
    ASSERT_EQ(counters[4].value, 1)
    ASSERT_EQ(counters[5].value, 1)
    ASSERT_EQ(counters[6].value, 1)
    ASSERT_EQ(counters[7].value, 1)
    ASSERT_EQ(counters[8].value, 1)
    ASSERT_EQ(counters[9].value, 1)
}

TEST(stop_multiple_timers)
{
    int timeouts[10] = {
        50, 0, 100, 75, 50, 50, 100, 90, 10, 0
    };
    struct counter_t counters[10];
    int i;
    struct async_t async;

    async_init(&async);

    for (i = 0; i < 10; i++) {
        counters[i].value = 0;
        async_timer_init(&counters[i].timer,
                         on_timeout,
                         timeouts[i],
                         0,
                         &async);
        mock_prepare_time_ms(0);
        async_timer_start(&counters[i].timer);
    }

    /* 0 ms timers expires. */
    mock_prepare_process(0, 1);
    ASSERT_EQ(async_process(&async), 9);
    ASSERT_EQ(counters[0].value, 0);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 0);
    ASSERT_EQ(counters[3].value, 0);
    ASSERT_EQ(counters[4].value, 0);
    ASSERT_EQ(counters[5].value, 0);
    ASSERT_EQ(counters[6].value, 0);
    ASSERT_EQ(counters[7].value, 0);
    ASSERT_EQ(counters[8].value, 0);
    ASSERT_EQ(counters[9].value, 1);

    /* Stop longest timers. */
    async_timer_stop(&counters[2].timer);
    async_timer_stop(&counters[6].timer);

    /* Stop a timer in the middle. */
    async_timer_stop(&counters[4].timer);

    /* Stop the first timer. */
    async_timer_stop(&counters[8].timer);

    /* Just before 50 ms timers expires. */
    mock_prepare_process(48, 48);
    ASSERT_EQ(async_process(&async), 2);
    ASSERT_EQ(counters[0].value, 0);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 0);
    ASSERT_EQ(counters[3].value, 0);
    ASSERT_EQ(counters[4].value, 0);
    ASSERT_EQ(counters[5].value, 0);
    ASSERT_EQ(counters[6].value, 0);
    ASSERT_EQ(counters[7].value, 0);
    ASSERT_EQ(counters[8].value, 0);
    ASSERT_EQ(counters[9].value, 1);

    /* 50 ms timers expires. */
    mock_prepare_process(50, 50);
    ASSERT_EQ(async_process(&async), 25);
    ASSERT_EQ(counters[0].value, 1);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 0);
    ASSERT_EQ(counters[3].value, 0);
    ASSERT_EQ(counters[4].value, 0);
    ASSERT_EQ(counters[5].value, 1);
    ASSERT_EQ(counters[6].value, 0);
    ASSERT_EQ(counters[7].value, 0);
    ASSERT_EQ(counters[8].value, 0);
    ASSERT_EQ(counters[9].value, 1);

    /* Just before 75 ms timer expire. */
    mock_prepare_process(74, -1);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_NO_ACTION);
    ASSERT_EQ(counters[0].value, 1);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 0);
    ASSERT_EQ(counters[3].value, 0);
    ASSERT_EQ(counters[4].value, 0);
    ASSERT_EQ(counters[5].value, 1);
    ASSERT_EQ(counters[6].value, 0);
    ASSERT_EQ(counters[7].value, 0);
    ASSERT_EQ(counters[8].value, 0);
    ASSERT_EQ(counters[9].value, 1);

    /* 75 ms timer expire. */
    mock_prepare_process(75, 75);
    ASSERT_EQ(async_process(&async), 15);
    ASSERT_EQ(counters[0].value, 1);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 0);
    ASSERT_EQ(counters[3].value, 1);
    ASSERT_EQ(counters[4].value, 0);
    ASSERT_EQ(counters[5].value, 1);
    ASSERT_EQ(counters[6].value, 0);
    ASSERT_EQ(counters[7].value, 0);
    ASSERT_EQ(counters[8].value, 0);
    ASSERT_EQ(counters[9].value, 1);

    /* 90 ms timer expire. */
    mock_prepare_process(90, -1);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_LAST_STOPPED);
    ASSERT_EQ(counters[0].value, 1);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 0);
    ASSERT_EQ(counters[3].value, 1);
    ASSERT_EQ(counters[4].value, 0);
    ASSERT_EQ(counters[5].value, 1);
    ASSERT_EQ(counters[6].value, 0);
    ASSERT_EQ(counters[7].value, 1);
    ASSERT_EQ(counters[8].value, 0);
    ASSERT_EQ(counters[9].value, 1);

    /* No 100 ms timer should expire. */
    mock_prepare_process(100, -1);
    ASSERT_EQ(async_process(&async), -ASYNC_ERROR_TIMER_NO_ACTION);
    ASSERT_EQ(counters[0].value, 1);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 0);
    ASSERT_EQ(counters[3].value, 1);
    ASSERT_EQ(counters[4].value, 0);
    ASSERT_EQ(counters[5].value, 1);
    ASSERT_EQ(counters[6].value, 0);
    ASSERT_EQ(counters[7].value, 1);
    ASSERT_EQ(counters[8].value, 0);
    ASSERT_EQ(counters[9].value, 1);
}
