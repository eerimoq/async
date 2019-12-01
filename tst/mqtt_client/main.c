#include "nala.h"
#include "nala_mocks.h"
#include "async.h"

static async_func_t tcp_on_connect_complete;
static async_func_t tcp_on_disconnected;
static async_func_t tcp_on_input;
static void *tcp_obj_p;

static void save_tcp_callbacks(struct async_tcp_client_t *self_p,
                               async_func_t on_connect_complete,
                               async_func_t on_disconnected,
                               async_func_t on_input,
                               void *obj_p,
                               struct async_t *async_p)
{
    (void)self_p;
    tcp_on_connect_complete = on_connect_complete;
    tcp_on_disconnected = on_disconnected;
    tcp_on_input = on_input;
    tcp_obj_p = obj_p;
    (void)async_p;
}

TEST(test_basic)
{
    struct async_t async;
    struct async_mqtt_client_t client;
    uint8_t connect[] = {
        0x10, 0x18, 0x00, 0x04, 0x4d, 0x51, 0x54, 0x54, 0x05, 0x00,
        0x00, 0x1e, 0x00, 0x00, 0x0b, 0x61, 0x73, 0x79, 0x6e, 0x63,
        0x2d, 0x31, 0x32, 0x33, 0x34, 0x35
    };

    async_tcp_client_init_mock_ignore_in_once();
    async_tcp_client_init_mock_set_callback(save_tcp_callbacks);

    async_init(&async);
    async_mqtt_client_init(&client,
                           "foo",
                           1883,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           &async);

    async_tcp_client_connect_mock_once("foo", 1883);
    async_mqtt_client_start(&client);

    async_tcp_client_is_connected_mock_once(true);
    async_tcp_client_write_mock_once(26, 26);
    async_tcp_client_write_mock_set_buf_p_in(&connect[0], sizeof(connect));
    tcp_on_connect_complete(tcp_obj_p);

    /* ToDo: Input connack and verify connected flag (and more). */
}
