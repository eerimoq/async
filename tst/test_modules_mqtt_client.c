#include "nala.h"
#include "nala_mocks.h"
#include "async.h"
#include "async/modules/mqtt_client.h"

static async_tcp_client_connected_t tcp_on_connected;
static async_tcp_client_disconnected_t tcp_on_disconnected;
static async_tcp_client_input_t tcp_on_input;
static struct async_tcp_client_t *tcp_p;

static void save_tcp_callbacks(struct async_tcp_client_t *self_p,
                               async_tcp_client_connected_t on_connected,
                               async_tcp_client_disconnected_t on_disconnected,
                               async_tcp_client_input_t on_input,
                               struct async_t *async_p)
{
    tcp_p = self_p;
    tcp_on_connected = on_connected;
    tcp_on_disconnected = on_disconnected;
    tcp_on_input = on_input;
    (void)async_p;
}

TEST(basic)
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

    async_tcp_client_write_mock_once(26, 26);
    async_tcp_client_write_mock_set_buf_p_in(&connect[0], sizeof(connect));
    tcp_on_connected(tcp_p, 0);

    /* ToDo: Input connack and verify connected flag (and more). */
}
