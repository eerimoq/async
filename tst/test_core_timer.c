#include "nala.h"
#include "async.h"
#include "utils.h"

struct counter_t {
    struct async_timer_t timer;
    int value;
};

static void on_timeout(struct counter_t *self_p)
{
    self_p->value++;
}

TEST(single_shot_timer)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     (async_timer_timeout_t)on_timeout,
                     &counter,
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

TEST(single_shot_timer_stop_running)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     (async_timer_timeout_t)on_timeout,
                     &counter,
                     0,
                     0,
                     &async);
    counter.value = 0;
    async_timer_start(&counter.timer);
    async_timer_stop(&counter.timer);
    async_destroy(&async);
}

TEST(single_shot_timer_stop_expired_before_handled)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     (async_timer_timeout_t)on_timeout,
                     &counter,
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

TEST(timer_get_and_set_initial_and_repeat)
{
    struct async_t async;
    struct async_timer_t timer;

    async_init(&async);
    async_timer_init(&timer,
                     NULL,
                     NULL,
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
                     (async_timer_timeout_t)on_timeout,
                     &counter,
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

TEST(restart_with_outstanding_timeout)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     (async_timer_timeout_t)on_timeout,
                     &counter,
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

TEST(restart_with_outstanding_timeouts)
{
    struct async_t async;
    struct counter_t counter;

    async_init(&async);
    async_timer_init(&counter.timer,
                     (async_timer_timeout_t)on_timeout,
                     &counter,
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

TEST(multiple_timers)
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
                         (async_timer_timeout_t)on_timeout,
                         &counters[i],
                         timeouts[i],
                         0,
                         &async);
        async_timer_start(&counters[i].timer);
    }

    /* 0 ms timers expires. */
    async_tick(&async);
    async_process(&async);
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

    /* 10 ms timer expire. */
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counters[0].value, 0);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 0);
    ASSERT_EQ(counters[3].value, 0);
    ASSERT_EQ(counters[4].value, 0);
    ASSERT_EQ(counters[5].value, 0);
    ASSERT_EQ(counters[6].value, 0);
    ASSERT_EQ(counters[7].value, 0);
    ASSERT_EQ(counters[8].value, 1);
    ASSERT_EQ(counters[9].value, 1);

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
    ASSERT_EQ(counters[0].value, 1);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 0);
    ASSERT_EQ(counters[3].value, 0);
    ASSERT_EQ(counters[4].value, 1);
    ASSERT_EQ(counters[5].value, 1);
    ASSERT_EQ(counters[6].value, 0);
    ASSERT_EQ(counters[7].value, 0);
    ASSERT_EQ(counters[8].value, 1);
    ASSERT_EQ(counters[9].value, 1);

    /* 75 ms timer expire. */
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counters[0].value, 1);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 0);
    ASSERT_EQ(counters[3].value, 1);
    ASSERT_EQ(counters[4].value, 1);
    ASSERT_EQ(counters[5].value, 1);
    ASSERT_EQ(counters[6].value, 0);
    ASSERT_EQ(counters[7].value, 0);
    ASSERT_EQ(counters[8].value, 1);
    ASSERT_EQ(counters[9].value, 1);

    /* 90 ms timer expire. */
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counters[0].value, 1);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 0);
    ASSERT_EQ(counters[3].value, 1);
    ASSERT_EQ(counters[4].value, 1);
    ASSERT_EQ(counters[5].value, 1);
    ASSERT_EQ(counters[6].value, 0);
    ASSERT_EQ(counters[7].value, 1);
    ASSERT_EQ(counters[8].value, 1);
    ASSERT_EQ(counters[9].value, 1);

    /* 100 ms timers expires. */
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
    ASSERT_EQ(counters[0].value, 1);
    ASSERT_EQ(counters[1].value, 1);
    ASSERT_EQ(counters[2].value, 1);
    ASSERT_EQ(counters[3].value, 1);
    ASSERT_EQ(counters[4].value, 1);
    ASSERT_EQ(counters[5].value, 1);
    ASSERT_EQ(counters[6].value, 1);
    ASSERT_EQ(counters[7].value, 1);
    ASSERT_EQ(counters[8].value, 1);
    ASSERT_EQ(counters[9].value, 1);
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
    async_set_tick_in_ms(&async, 5);

    for (i = 0; i < 10; i++) {
        counters[i].value = 0;
        async_timer_init(&counters[i].timer,
                         (async_timer_timeout_t)on_timeout,
                         &counters[i],
                         timeouts[i],
                         0,
                         &async);
        async_timer_start(&counters[i].timer);
    }

    /* 0 ms timers expires. */
    async_tick(&async);
    async_process(&async);
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
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
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
    async_tick(&async);
    async_process(&async);
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
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
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
    async_tick(&async);
    async_process(&async);
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
    async_tick(&async);
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
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
    async_tick(&async);
    async_tick(&async);
    async_process(&async);
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
