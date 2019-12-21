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

static int on_connected_count;

static void on_connected(void *obj_p)
{
    (void)obj_p;

    on_connected_count++;
}

TEST(basic)
{
    struct async_t async;
    struct async_mqtt_client_t client;
    uint8_t connect[] = {
        0x10, 0x18, 0x00, 0x04, 0x4d, 0x51, 0x54, 0x54, 0x05, 0x02,
        0x00, 0x1e, 0x00, 0x00, 0x0b, 0x61, 0x73, 0x79, 0x6e, 0x63,
        0x2d, 0x31, 0x32, 0x33, 0x34, 0x35
    };
    uint8_t connack[] = {
        0x20, 0x0b, 0x00, 0x00, 0x08, 0x24, 0x00, 0x25, 0x00, 0x28,
        0x00, 0x2a, 0x00
    };

    on_connected_count = 0;

    async_tcp_client_init_mock_ignore_in_once();
    async_tcp_client_init_mock_set_callback(save_tcp_callbacks);

    async_init(&async);
    async_mqtt_client_init(&client,
                           "foo",
                           1883,
                           on_connected,
                           NULL,
                           NULL,
                           NULL,
                           &async);

    async_tcp_client_connect_mock_once("foo", 1883);
    async_mqtt_client_start(&client);

    async_tcp_client_write_mock_once(26);
    async_tcp_client_write_mock_set_buf_p_in(&connect[0], sizeof(connect));
    tcp_on_connected(tcp_p, 0);

    /* CONNACK: Fixed header. */
    async_tcp_client_read_mock_once(1, 1);
    async_tcp_client_read_mock_set_buf_p_out(&connack[0], 1);
    tcp_on_input(tcp_p);
    async_tcp_client_read_mock_once(1, 1);
    async_tcp_client_read_mock_set_buf_p_out(&connack[1], 1);
    tcp_on_input(tcp_p);

    /* CONNACK: Data. */
    async_tcp_client_read_mock_once(11, 11);
    async_tcp_client_read_mock_set_buf_p_out(&connack[2], 11);
    tcp_on_input(tcp_p);

    ASSERT_EQ(on_connected_count, 1)
}

TEST(connect_will)
{
    struct async_t async;
    struct async_mqtt_client_t client;
    const char will_topic[] = "foo";
    uint8_t will_message[] = { 'b', 'a', 'r' };
    /* Connect with will topic 'foo' and message 'bar'. */
    uint8_t connect[] = {
        0x10, 0x23, 0x00, 0x04, 0x4d, 0x51, 0x54, 0x54, 0x05, 0x06,
        0x00, 0x1e, 0x00, 0x00, 0x0b, 0x61, 0x73, 0x79, 0x6e, 0x63,
        0x2d, 0x31, 0x32, 0x33, 0x34, 0x35, 0x00, 0x00, 0x03, 0x66,
        0x6f, 0x6f, 0x00, 0x03, 0x62, 0x61, 0x72
    };

    async_tcp_client_init_mock_ignore_in_once();
    async_tcp_client_init_mock_set_callback(save_tcp_callbacks);

    async_init(&async);
    async_mqtt_client_init(&client,
                           "host",
                           1883,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           &async);
    async_mqtt_client_set_will(&client,
                               &will_topic[0],
                               &will_message[0],
                               sizeof(will_message));

    async_tcp_client_connect_mock_once("host", 1883);
    async_mqtt_client_start(&client);

    async_tcp_client_write_mock_once(37);
    async_tcp_client_write_mock_set_buf_p_in(&connect[0], sizeof(connect));
    tcp_on_connected(tcp_p, 0);
}
