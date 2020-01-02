#include <sys/eventfd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dbg.h>
#include "nala.h"
#include "nala_mocks.h"
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
    (void)tcp_p;

    ASSERT_EQ(res, -1);
    exit(0);
}

static void do_connect(struct async_tcp_client_t *self_p)
{
    async_tcp_client_connect(self_p, "localhost", 9999);
}

TEST(tcp_client_connect_failure)
{
    struct async_t async;
    struct async_tcp_client_t tcp;
    struct sockaddr_in addr;
    int sockfd;

    sockfd = 6;
    socket_mock_once(AF_INET, SOCK_STREAM, 0, sockfd);
    connect_mock_once(sockfd, sizeof(addr), -1);
    async_init(&async);
    async_set_runtime(&async, async_runtime_create());
    async_tcp_client_init(&tcp,
                          on_tcp_connected,
                          NULL,
                          NULL,
                          &async);
    async_call(&async, (async_func_t)do_connect, &tcp);
    async_run_forever(&async);
}
