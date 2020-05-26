#include "nala.h"
#include "async.h"
#include "runtime_test.h"

TEST(call_all_functions)
{
    struct async_t async;
    struct async_tcp_server_t server;
    struct async_tcp_server_client_t client;

    async_init(&async);

    runtime_test_set_async_mock();
    async_set_runtime(&async, runtime_test_create());

    runtime_test_tcp_server_init_mock_once("127.0.0.1", 4444);
    async_tcp_server_init(&server, "127.0.0.1", 4444, NULL, NULL, NULL, &async);

    runtime_test_tcp_server_start_mock_once();
    async_tcp_server_start(&server);

    runtime_test_tcp_server_stop_mock_once();
    async_tcp_server_stop(&server);

    runtime_test_tcp_server_add_client_mock_once();
    async_tcp_server_add_client(&server, &client);

    runtime_test_tcp_server_client_write_mock_once(5);
    async_tcp_server_client_write(&client, NULL, 5);

    runtime_test_tcp_server_client_read_mock_once(5, 6);
    ASSERT_EQ(async_tcp_server_client_read(&client, NULL, 5), 6u);

    runtime_test_tcp_server_client_disconnect_mock_once();
    async_tcp_server_client_disconnect(&client);
}
