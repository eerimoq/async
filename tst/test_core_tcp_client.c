#include "nala.h"
#include "async.h"
#include "runtime_test.h"

TEST(call_all_functions)
{
    struct async_t async;
    struct async_tcp_client_t tcp;

    async_init(&async);

    runtime_test_set_async_mock();
    async_set_runtime(&async, runtime_test_create());

    runtime_test_tcp_client_init_mock_once();
    async_tcp_client_init(&tcp, NULL, NULL, NULL, &async);

    runtime_test_tcp_client_connect_mock_once("foo", 5);
    async_tcp_client_connect(&tcp, "foo", 5);

    runtime_test_tcp_client_write_mock_once(3);
    async_tcp_client_write(&tcp, NULL, 3);

    runtime_test_tcp_client_read_mock_once(5, 6);
    ASSERT_EQ(async_tcp_client_read(&tcp, NULL, 5), 6u);

    runtime_test_tcp_client_disconnect_mock_once();
    async_tcp_client_disconnect(&tcp);
}

TEST(call_default_callbacks)
{
    struct async_t async;
    struct async_tcp_client_t tcp;
    int handle;
    struct nala_runtime_test_tcp_client_init_params_t *params_p;

    async_init(&async);

    runtime_test_set_async_mock();
    async_set_runtime(&async, runtime_test_create());

    handle = runtime_test_tcp_client_init_mock_once();
    async_tcp_client_init(&tcp, NULL, NULL, NULL, &async);

    runtime_test_tcp_client_connect_mock_once("bar", 51);
    async_tcp_client_connect(&tcp, "bar", 51);

    params_p = runtime_test_tcp_client_init_mock_get_params_in(handle);

    /* Call the callbacks. */
    params_p->on_connected(&tcp, 0);
    runtime_test_tcp_client_read_mock_once(32, 30);
    runtime_test_tcp_client_read_mock_once(32, 0);
    params_p->on_input(&tcp);
    params_p->on_disconnected(&tcp);
}
