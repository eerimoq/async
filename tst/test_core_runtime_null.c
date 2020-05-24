#include "nala.h"
#include "async.h"
#include "subprocess.h"

static void assert_exit_1_and_output(void (*entry)(), const char *expected_p)
{
    struct subprocess_result_t *result_p;

    result_p = subprocess_call_output(entry, NULL);
    ASSERT_EQ(result_p->exit_code, 1);
    ASSERT_EQ(result_p->stderr.buf_p, expected_p);
    subprocess_result_free(result_p);
}

static void set_default_runtime_entry()
{
    async_runtime_null_create()->set_async(NULL, NULL);
}

TEST(set_default_runtime)
{
    assert_exit_1_and_output(set_default_runtime_entry,
                             "async_set_runtime() not implemented.\n");
}

static void call_threadsafe_entry()
{
    async_runtime_null_create()->call_threadsafe(NULL, NULL, NULL, NULL);
}

TEST(call_threadsafe)
{
    assert_exit_1_and_output(call_threadsafe_entry,
                             "async_call_threadsafe() not implemented.\n");
}

static void call_worker_pool_entry()
{
    async_runtime_null_create()->call_worker_pool(NULL, NULL, NULL, NULL, NULL);
}

TEST(call_worker_pool)
{
    assert_exit_1_and_output(call_worker_pool_entry,
                             "async_call_worker_pool() not implemented.\n");
}

static void run_forever_entry()
{
    async_runtime_null_create()->run_forever(NULL);
}

TEST(run_forever)
{
    assert_exit_1_and_output(run_forever_entry,
                             "async_run_forever() not implemented.\n");
}

static void tcp_client_init_entry()
{
    async_runtime_null_create()->tcp_client.init(NULL, NULL, NULL, NULL);
}

TEST(tcp_client_init)
{
    assert_exit_1_and_output(tcp_client_init_entry,
                             "async_tcp_client_init() not implemented.\n");
}

static void tcp_client_connect_entry()
{
    async_runtime_null_create()->tcp_client.connect(NULL, "", 0);
}

TEST(tcp_client_connect)
{
    assert_exit_1_and_output(tcp_client_connect_entry,
                             "async_tcp_client_connect() not implemented.\n");
}

static void tcp_client_disconnect_entry()
{
    async_runtime_null_create()->tcp_client.disconnect(NULL);
}

TEST(tcp_client_disconnect)
{
    assert_exit_1_and_output(tcp_client_disconnect_entry,
                             "async_tcp_client_disconnect() not implemented.\n");
}

static void tcp_client_write_entry()
{
    async_runtime_null_create()->tcp_client.write(NULL, NULL, 0);
}

TEST(tcp_client_write)
{
    assert_exit_1_and_output(tcp_client_write_entry,
                             "async_tcp_client_write() not implemented.\n");
}

static void tcp_client_read_entry()
{
    async_runtime_null_create()->tcp_client.read(NULL, NULL, 0);
}

TEST(tcp_client_read)
{
    assert_exit_1_and_output(tcp_client_read_entry,
                             "async_tcp_client_read() not implemented.\n");
}

static void tcp_server_init_entry()
{
    async_runtime_null_create()->tcp_server.init(NULL);
}

TEST(tcp_server_init)
{
    assert_exit_1_and_output(tcp_server_init_entry,
                             "async_tcp_server_init() not implemented.\n");
}

static void tcp_server_start_entry()
{
    async_runtime_null_create()->tcp_server.start(NULL);
}

TEST(tcp_server_start)
{
    assert_exit_1_and_output(tcp_server_start_entry,
                             "async_tcp_server_start() not implemented.\n");
}

static void tcp_server_stop_entry()
{
    async_runtime_null_create()->tcp_server.stop(NULL);
}

TEST(tcp_server_stop)
{
    assert_exit_1_and_output(tcp_server_stop_entry,
                             "async_tcp_server_stop() not implemented.\n");
}

static void tcp_server_client_write_entry()
{
    async_runtime_null_create()->tcp_server.client.write(NULL, NULL, 0);
}

TEST(tcp_server_client_write)
{
    assert_exit_1_and_output(tcp_server_client_write_entry,
                             "async_tcp_server_client_write() not implemented.\n");
}

static void tcp_server_client_read_entry()
{
    async_runtime_null_create()->tcp_server.client.read(NULL, NULL, 0);
}

TEST(tcp_server_client_read)
{
    assert_exit_1_and_output(tcp_server_client_read_entry,
                             "async_tcp_server_client_read() not implemented.\n");
}

static void tcp_server_client_disconnect_entry()
{
    async_runtime_null_create()->tcp_server.client.disconnect(NULL);
}

TEST(tcp_server_client_disconnect)
{
    assert_exit_1_and_output(
        tcp_server_client_disconnect_entry,
        "async_tcp_server_client_disconnect() not implemented.\n");
}
