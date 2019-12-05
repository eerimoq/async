#include "nala.h"
#include "nala_mocks.h"
#include "async.h"
#include "runtime_test.h"

TEST(test_call_all_functions)
{
    struct async_t async;
    struct async_tcp_client_t tcp;

    async_init(&async);

    runtime_test_set_async_mock();
    async_set_runtime(&async, runtime_test_create());

    runtime_test_tcp_client_init_mock_once(NULL, NULL, NULL);
    async_tcp_client_init(&tcp, NULL, NULL, NULL, &async);

    runtime_test_tcp_client_connect_mock_once("foo", 5);
    async_tcp_client_connect(&tcp, "foo", 5);

    runtime_test_tcp_client_write_mock_once(3, 4);
    ASSERT_EQ(async_tcp_client_write(&tcp, NULL, 3), 4);

    runtime_test_tcp_client_read_mock_once(5, 6);
    ASSERT_EQ(async_tcp_client_read(&tcp, NULL, 5), 6u);

    runtime_test_tcp_client_disconnect_mock_once();
    async_tcp_client_disconnect(&tcp);
}
