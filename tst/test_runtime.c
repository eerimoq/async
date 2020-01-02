#include <pthread.h>
#include "nala.h"
#include "async.h"

static bool single_shot_timer_expired = false;
static int periodic_timer_expiry_count = 0;

static void check_timers_test_done()
{
    if (single_shot_timer_expired && (periodic_timer_expiry_count == 2)) {
        exit(0);
    }
}

static void on_single_shot_timer_expiry()
{
    if (single_shot_timer_expired) {
        FAIL();
    }

    single_shot_timer_expired = true;
    check_timers_test_done();
}

static void on_periodic_timer_expiry()
{
    periodic_timer_expiry_count++;
    check_timers_test_done();
}

TEST(timers)
{
    struct async_t async;
    struct async_timer_t timers[2];

    async_init(&async);
    async_set_runtime(&async, async_runtime_create());
    async_timer_init(&timers[0], on_single_shot_timer_expiry, 1, 0, &async);
    async_timer_start(&timers[0]);
    async_timer_init(&timers[1], on_periodic_timer_expiry, 0, 1, &async);
    async_timer_start(&timers[1]);
    async_run_forever(&async);
}

static void on_tcp_connected(struct async_tcp_client_t *tcp_p, int res)
{
    ASSERT_NE(tcp_p, NULL);
    ASSERT_EQ(res, -1);
    exit(0);
}

TEST(tcp_client_connect_failure)
{
    struct async_t async;
    struct async_tcp_client_t tcp;

    async_init(&async);
    async_set_runtime(&async, async_runtime_create());
    async_tcp_client_init(&tcp,
                          on_tcp_connected,
                          NULL,
                          NULL,
                          &async);
    async_tcp_client_connect(&tcp, "async-localhost", 9999);
    async_run_forever(&async);
}

static bool hello_called = false;

static void hello(void *obj_p)
{
    ASSERT_EQ(obj_p, NULL);
    hello_called = true;
}

static void on_complete(void *obj_p)
{
    ASSERT_EQ(obj_p, NULL);
    ASSERT(hello_called);
    exit(0);
}

TEST(call_worker_pool)
{
    struct async_t async;

    async_init(&async);
    async_set_runtime(&async, async_runtime_create());
    async_call_worker_pool(&async, hello, NULL, on_complete);
    async_run_forever(&async);
}

static pthread_t threadsafe_caller_pthread;

static void called_in_async_thread(void *obj_p)
{
    ASSERT_EQ(obj_p, NULL);
    exit(0);
}

static void *threadsafe_caller(struct async_t *async_p)
{
    async_call_threadsafe(async_p, called_in_async_thread, NULL);

    return (NULL);
}

TEST(call_threadsafe)
{
    struct async_t async;

    async_init(&async);
    async_set_runtime(&async, async_runtime_create());
    pthread_create(&threadsafe_caller_pthread,
                   NULL,
                   (void *(*)(void *))threadsafe_caller,
                   &async);
    async_run_forever(&async);
}
