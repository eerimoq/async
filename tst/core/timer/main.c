#include "nala.h"
#include "nala_mocks.h"
#include "async.h"

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

TEST(test_single_shot_timer)
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
    async_timer_start(&counter.timer);
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 1);
    async_destroy(&async);
}

TEST(test_single_shot_timer_stop_running)
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
    async_timer_start(&counter.timer);
    async_timer_stop(&counter.timer);
    async_destroy(&async);
}

TEST(test_single_shot_timer_stop_expired_before_handled)
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
    async_timer_start(&counter.timer);
    async_tick(&async);
    async_timer_stop(&counter.timer);
    async_process(&async);
    ASSERT_EQ(counter.value, 0);
    async_destroy(&async);
}

TEST(test_timer_get_and_set_initial_and_repeat)
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

TEST(test_initial_and_repeat)
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
    async_timer_start(&counter.timer);

    /* Initial timeout. */
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 1);

    /* Repeated timeout 1. */
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 2);

    /* Repeated timeout 2. */
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 3);

    /* Repeated timeout 3. */
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 4);

    /* Change repeat. Takes effect after current expiry. */
    async_timer_set_repeat(&counter.timer, 200);

    /* Repeated timeout 4. */
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 5);

    /* Repeated timeout 5. */
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 6);

    /* Repeated timeout 6. */
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 7);

    /* Stop the timer, set initial and repeat and start it again. */
    async_timer_stop(&counter.timer);
    async_timer_set_initial(&counter.timer, 0);
    async_timer_set_repeat(&counter.timer, 100);
    async_timer_start(&counter.timer);

    /* Inital timeout. */
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 8);

    /* Repeated timeout 1. */
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 9);

    /* Repeated timeout 2. */
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 10);

    async_destroy(&async);
}

TEST(test_restart_with_outstanding_timeout)
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
    async_timer_start(&counter.timer);
    async_tick(&async);
    async_timer_stop(&counter.timer);
    async_timer_start(&counter.timer);
    async_process(&async);
    ASSERT_EQ(counter.value, 0);
    async_destroy(&async);
}

TEST(test_restart_with_outstanding_timeouts)
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

    /* Ignore 5 timeouts.  */
    async_timer_start(&counter.timer);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_timer_stop(&counter.timer);
    async_process(&async);

    /* Start again and count 3 timeouts. */
    async_timer_start(&counter.timer);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counter.value, 3);

    async_destroy(&async);
}

TEST(test_multiple_timers)
{
    int timeouts[10] = {
        50, 0, 100, 75, 50, 50, 100, 90, 10, 0
    };
    struct counter_t counters[10];
    int i;
    struct async_t async;

    async_init(&async);
    async_set_tick_in_ms(&async, 5);

    for (i = 0; i < 10; i++) {
        counters[i].value = 0;
        async_timer_init(&counters[i].timer,
                         on_timeout,
                         timeouts[i],
                         0,
                         &async);
        async_timer_start(&counters[i].timer);
    }

    /* 0 ms timers expires. */
    async_tick(&async);
    async_process(&async);
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
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
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
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
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
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
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
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
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
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
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
