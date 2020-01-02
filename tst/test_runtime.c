#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
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

static void do_connect(struct async_tcp_client_t *tcp_p)
{
    async_tcp_client_connect(tcp_p, "localhost", 9999);
}

static void tcp_client_server_initiated_close_on_connected(
    struct async_tcp_client_t *tcp_p, int res)
{
    if (res == 0) {
        async_tcp_client_write(tcp_p, "1", 1);
    } else {
        usleep(1000);
        do_connect(tcp_p);
    }
}

static void tcp_client_server_initiated_close_on_disconnected(
    struct async_tcp_client_t *tcp_p)
{
    (void)tcp_p;
    exit(0);
}

static void tcp_client_server_initiated_close_on_input(
    struct async_tcp_client_t *self_p)
{
    char ch;

    async_tcp_client_read(self_p, &ch, 1);
    ASSERT_EQ(ch, '1');
}

static void *tcp_client_server_initiated_close_server_main(void *arg_p)
{
    (void)arg_p;

    int sock;
    struct sockaddr_in addr;
    char ch;
    int yes;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    inet_aton("127.0.0.1", (struct in_addr *)&addr.sin_addr.s_addr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    yes = 1;
    ASSERT_EQ(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)), 0);
    ASSERT_EQ(bind(sock, &addr, sizeof(addr)), 0);
    ASSERT_EQ(listen(sock, 5), 0);
    sock = accept(sock, NULL, 0);
    ASSERT_EQ(read(sock, &ch, 1), 1);
    ASSERT_EQ(write(sock, &ch, 1), 1);
    ASSERT_EQ(close(sock), 0);

    return (NULL);
}

TEST(tcp_client_server_initiated_close)
{
    struct async_t async;
    struct async_tcp_client_t tcp;
    pthread_t server_pthread;

    pthread_create(&server_pthread,
                   NULL,
                   tcp_client_server_initiated_close_server_main,
                   NULL);

    async_init(&async);
    async_set_runtime(&async, async_runtime_create());
    async_tcp_client_init(&tcp,
                          tcp_client_server_initiated_close_on_connected,
                          tcp_client_server_initiated_close_on_disconnected,
                          tcp_client_server_initiated_close_on_input,
                          &async);
    async_call(&async, (async_func_t)do_connect, &tcp);
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
