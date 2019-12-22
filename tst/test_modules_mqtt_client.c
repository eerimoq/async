#include "nala.h"
#include "nala_mocks.h"
#include "async.h"
#include "async/modules/mqtt_client.h"
#include "utils.h"

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
    (void)async_p;

    tcp_p = self_p;
    tcp_on_connected = on_connected;
    tcp_on_disconnected = on_disconnected;
    tcp_on_input = on_input;
}

static void on_connected(void *obj_p)
{
    mqtt_on_connected(obj_p);
}

static void on_publish(void *obj_p,
                       const char *topic_p,
                       const uint8_t *buf_p,
                       size_t size)
{
    mqtt_on_publish(obj_p, topic_p, buf_p, size);
}

static void on_subscribe_complete(void *obj_p, uint16_t transaction_id)
{
    mqtt_on_subscribe_complete(obj_p, transaction_id);
}

static void assert_init(struct async_t *async_p,
                        struct async_mqtt_client_t *client_p)
{
    async_tcp_client_init_mock_ignore_in_once();
    async_tcp_client_init_mock_set_callback(save_tcp_callbacks);

    async_init(async_p);
    async_mqtt_client_init(client_p,
                           "foo",
                           1883,
                           on_connected,
                           NULL,
                           on_publish,
                           NULL,
                           async_p);
}

static void assert_start_and_on_tcp_connected(
    struct async_mqtt_client_t *client_p,
    uint8_t *connect_p,
    size_t size)
{
    async_tcp_client_connect_mock_once("foo", 1883);
    async_mqtt_client_start(client_p);

    async_tcp_client_write_mock_once(size);
    async_tcp_client_write_mock_set_buf_p_in(connect_p, size);
    tcp_on_connected(tcp_p, 0);
}

static void input_packet(uint8_t *buf_p,
                         size_t length_size,
                         size_t size)
{
    /* Fixed header. */
    async_tcp_client_read_mock_once(1, 1);
    async_tcp_client_read_mock_set_buf_p_out(&buf_p[0], 1);
    tcp_on_input(tcp_p);

    if (length_size != 1) {
        FAIL();
    }

    async_tcp_client_read_mock_once(length_size, length_size);
    async_tcp_client_read_mock_set_buf_p_out(&buf_p[1], length_size);
    tcp_on_input(tcp_p);

    /* Data. */
    size -= (length_size + 1);

    if (size > 0) {
        async_tcp_client_read_mock_once(size, size);
        async_tcp_client_read_mock_set_buf_p_out(&buf_p[length_size + 1], size);
        tcp_on_input(tcp_p);
    }
}

static void input_packet_suback(uint16_t transaction_id)
{
    uint8_t suback[] = {
        0x90, 0x04, 0x00, transaction_id, 0x00, 0x00
    };

    input_packet(&suback[0], 1, sizeof(suback));
}

static void input_packet_publish(void)
{
    uint8_t publish[] = {
        0x30, 0x0b, 0x00, 0x06, 'b', 'a', 'r', 'f', 'o', 'o',
        0x00, 0x56, 0x78
    };

    input_packet(&publish[0], 1, sizeof(publish));
}

static void assert_on_connected(uint8_t *connack_p,
                                size_t length_size,
                                size_t size)
{
    mqtt_on_connected_mock_once();
    input_packet(connack_p, length_size, size);
}

static void assert_start_until_connected(struct async_mqtt_client_t *client_p)
{
    uint8_t connect[] = {
        0x10, 0x18, 0x00, 0x04, 0x4d, 0x51, 0x54, 0x54, 0x05, 0x02,
        0x00, 0x1e, 0x00, 0x00, 0x0b, 0x61, 0x73, 0x79, 0x6e, 0x63,
        0x2d, 0x31, 0x32, 0x33, 0x34, 0x35
    };
    uint8_t connack[] = {
        0x20, 0x0b, 0x00, 0x00, 0x08, 0x24, 0x00, 0x25, 0x00, 0x28,
        0x00, 0x2a, 0x00
    };

    assert_start_and_on_tcp_connected(client_p,
                                      &connect[0],
                                      sizeof(connect));
    assert_on_connected(&connack[0], 1, sizeof(connack));
}

static void assert_until_connected(struct async_t *async_p,
                                   struct async_mqtt_client_t *client_p)
{
    assert_init(async_p, client_p);
    assert_start_until_connected(client_p);
}

static void mock_prepare_stop(void)
{
    uint8_t disconnect[] = {
        0xe0, 0x02, 0x00, 0x00
    };

    async_tcp_client_write_mock_once(sizeof(disconnect));
    async_tcp_client_write_mock_set_buf_p_in(&disconnect[0], sizeof(disconnect));
    async_tcp_client_disconnect_mock_once(tcp_p);
}

static void assert_stop(struct async_mqtt_client_t *client_p)
{
    mock_prepare_stop();
    async_mqtt_client_stop(client_p);
}

static void mock_prepare_subscribe_default(void)
{
    uint8_t subscribe[] = {
        0x80, 0x09, 0x00, 0x01, 0x00, 0x00, 0x03, 0x74, 0x74, 0x74,
        0x00
    };

    async_tcp_client_write_mock_once(sizeof(subscribe));
    async_tcp_client_write_mock_set_buf_p_in(&subscribe[0], sizeof(subscribe));
}

static void mock_prepare_publish_default(void)
{
    uint8_t publish[] = {
        0x30, 0x0b, 0x00, 0x06, 'f', 'o', 'o', 'b', 'a', 'r',
        0x00, 0x12, 0x34
    };

    async_tcp_client_write_mock_once(sizeof(publish));
    async_tcp_client_write_mock_set_buf_p_in(&publish[0], sizeof(publish));
}

TEST(init_start_stop)
{
    struct async_t async;
    struct async_mqtt_client_t client;

    assert_until_connected(&async, &client);
    assert_stop(&client);
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
    uint8_t connack[] = {
        0x20, 0x0b, 0x00, 0x00, 0x08, 0x24, 0x00, 0x25, 0x00, 0x28,
        0x00, 0x2a, 0x00
    };

    assert_init(&async, &client);
    async_mqtt_client_set_will(&client,
                               &will_topic[0],
                               &will_message[0],
                               sizeof(will_message));
    assert_start_and_on_tcp_connected(&client,
                                      &connect[0],
                                      sizeof(connect));
    assert_on_connected(&connack[0], 1, sizeof(connack));
    assert_stop(&client);
}

TEST(subscribe)
{
    struct async_t async;
    struct async_mqtt_client_t client;
    uint16_t transaction_id;

    assert_init(&async, &client);
    async_mqtt_client_set_on_subscribe_complete(&client, on_subscribe_complete);
    assert_start_until_connected(&client);

    /* SUBSCRIBE. */
    mock_prepare_subscribe_default();
    transaction_id = 1;
    ASSERT_EQ(async_mqtt_client_subscribe(&client, "ttt"), transaction_id);

    /* SUBACK. */
    mqtt_on_subscribe_complete_mock_once(transaction_id);
    input_packet_suback(transaction_id);

    assert_stop(&client);
}

TEST(subscribe_error_short_suback)
{
    struct async_t async;
    struct async_mqtt_client_t client;
    uint16_t transaction_id;
    uint8_t suback[] = {
        0x90, 0x00
    };

    assert_init(&async, &client);
    async_mqtt_client_set_on_subscribe_complete(&client, on_subscribe_complete);
    assert_start_until_connected(&client);

    /* SUBSCRIBE. */
    mock_prepare_subscribe_default();
    transaction_id = 1;
    ASSERT_EQ(async_mqtt_client_subscribe(&client, "ttt"), transaction_id);

    /* Short SUBACK. */
    input_packet(&suback[0], 1, sizeof(suback));
    assert_stop(&client);
}

TEST(publish)
{
    struct async_t async;
    struct async_mqtt_client_t client;
    uint8_t message[] = {
         0x12, 0x34
    };

    assert_init(&async, &client);
    assert_start_until_connected(&client);
    mock_prepare_publish_default();
    async_mqtt_client_publish(&client,
                              "foobar",
                              &message,
                              sizeof(message));
    assert_stop(&client);
}

TEST(receive_publish)
{
    struct async_t async;
    struct async_mqtt_client_t client;
    uint8_t message[] = { 0x56, 0x78 };

    assert_until_connected(&async, &client);
    mqtt_on_publish_mock_once("barfoo", 2);
    mqtt_on_publish_mock_set_buf_p_in(&message[0], sizeof(message));
    input_packet_publish();
    assert_stop(&client);
}

TEST(receive_publish_no_data)
{
    struct async_t async;
    struct async_mqtt_client_t client;
    uint8_t publish[] = {
        0x30, 0x09, 0x00, 0x06, 'b', 'a', 'r', 'f', 'o', 'o',
        0x00
    };

    assert_until_connected(&async, &client);
    mqtt_on_publish_mock_once("barfoo", 0);
    input_packet(&publish[0], 1, sizeof(publish));
    assert_stop(&client);
}

TEST(receive_publish_error_short_message_no_props)
{
    struct async_t async;
    struct async_mqtt_client_t client;
    uint8_t publish[] = {
        0x30, 0x08, 0x00, 0x06, 'b', 'a', 'r', 'f', 'o', 'o'
    };

    assert_until_connected(&async, &client);
    input_packet(&publish[0], 1, sizeof(publish));
    assert_stop(&client);
}