/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019, Erik Moqvist
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This file is part of the Async project.
 */

#include <stdio.h>
#include <string.h>
#include "asyncio.h"
#include "internal.h"
#include "bitstream.h"

/* Connection flags. */
#define CLEAN_START     0x02
#define WILL_FLAG       0x04
#define WILL_QOS_1      0x08
#define WILL_QOS_2      0x10
#define WILL_RETAIN     0x20
#define PASSWORD_FLAG   0x40
#define USER_NAME_FLAG  0x80

enum packet_state_t {
    packet_state_read_type_t = 0,
    packet_state_read_size_t,
    packet_state_read_data_t
};

/* Control packet types. */
enum control_packet_type_t {
    control_packet_type_connect_t = 1,
    control_packet_type_connack_t = 2,
    control_packet_type_publish_t = 3,
    control_packet_type_puback_t = 4,
    control_packet_type_pubrec_t = 5,
    control_packet_type_pubrel_t = 6,
    control_packet_type_pubcomp_t = 7,
    control_packet_type_subscribe_t = 8,
    control_packet_type_suback_t = 9,
    control_packet_type_unsubscribe_t = 10,
    control_packet_type_unsuback_t = 11,
    control_packet_type_pingreq_t = 12,
    control_packet_type_pingresp_t = 13,
    control_packet_type_disconnect_t = 14,
    control_packet_type_auth_t = 15
};

enum connect_reason_code_t {
    connect_reason_code_SUCCESS_t = 0,
    connect_reason_code_v3_1_1_unacceptable_protocol_version_t = 1,
    connect_reason_code_v3_1_1_identifier_rejected_t = 2,
    connect_reason_code_v3_1_1_server_unavailable_t = 3,
    connect_reason_code_v3_1_1_bad_user_name_or_password_t = 4,
    connect_reason_code_v3_1_1_not_authorized_t = 5,
    connect_reason_code_unspecified_error_t = 128,
    connect_reason_code_malformed_packet_t = 129,
    connect_reason_code_protocol_error_t = 130,
    connect_reason_code_implementation_specific_error_t = 131,
    connect_reason_code_unsupported_protocol_version_t = 132,
    connect_reason_code_client_identifier_not_valid_t = 133,
    connect_reason_code_bad_user_name_or_password_t = 134,
    connect_reason_code_not_authorized_t = 135,
    connect_reason_code_server_unavailable_t = 136,
    connect_reason_code_server_busy_t = 137,
    connect_reason_code_banned_t = 138,
    connect_reason_code_bad_authentication_method_t = 140,
    connect_reason_code_topic_name_invalid_t = 144,
    connect_reason_code_packet_too_large_t = 149,
    connect_reason_code_quota_exceeded_t = 151,
    connect_reason_code_payload_format_invalid_t = 153,
    connect_reason_code_retain_not_supported_t = 154,
    connect_reason_code_qos_not_supported_t = 155,
    connect_reason_code_use_another_server_t = 156,
    connect_reason_code_server_moved_t = 157,
    connect_reason_code_connection_rate_exceeded_t = 159
};

enum disconnect_reason_code_t {
    disconnect_reason_code_normal_disconnection_t = 0,
    disconnect_reason_code_disconnect_with_will_message_t = 4,
    disconnect_reason_code_unspecified_error_t = 128,
    disconnect_reason_code_malformed_packet_t = 129,
    disconnect_reason_code_protocol_error_t = 130,
    disconnect_reason_code_implementation_specific_error_t = 131,
    disconnect_reason_code_not_authorized_t = 135,
    disconnect_reason_code_server_busy_t = 137,
    disconnect_reason_code_server_shutting_down_t = 139,
    disconnect_reason_code_keep_alive_timeout_t = 141,
    disconnect_reason_code_session_taken_over_t = 142,
    disconnect_reason_code_topic_filter_invalid_t = 143,
    disconnect_reason_code_topic_name_invalid_t = 144,
    disconnect_reason_code_receive_maximum_exceeded_t = 147,
    disconnect_reason_code_topic_alias_invalid_t = 148,
    disconnect_reason_code_packet_too_large_t = 149,
    disconnect_reason_code_message_rate_too_high_t = 150,
    disconnect_reason_code_quota_exceeded_t = 151,
    disconnect_reason_code_administrative_action_t = 152,
    disconnect_reason_code_payload_format_invalid_t = 153,
    disconnect_reason_code_retain_not_supported_t = 154,
    disconnect_reason_code_qos_not_supported_t = 155,
    disconnect_reason_code_use_another_server_t = 156,
    disconnect_reason_code_server_moved_t = 157,
    disconnect_reason_code_shared_subscriptions_not_supported_t = 158,
    disconnect_reason_code_connection_rate_exceeded_t = 159,
    disconnect_reason_code_maximum_connect_time_t = 160,
    disconnect_reason_code_subscription_identifiers_not_supported_t = 161,
    disconnect_reason_code_wildcard_subscriptions_not_supported_t = 162
};

enum suback_reason_code_t {
    subsck_reason_code_granted_qos_0_t = 0,
    subsck_reason_code_granted_qos_1_t = 1,
    subsck_reason_code_granted_qos_2_t = 2,
    subsck_reason_code_unspecified_error_t = 128,
    subsck_reason_code_implementation_specific_error_t = 131,
    subsck_reason_code_not_authorized_t = 135,
    subsck_reason_code_topic_filter_invalid_t = 143,
    subsck_reason_code_packet_identifier_in_use_t = 145,
    subsck_reason_code_quota_exceeded_t = 151,
    subsck_reason_code_shared_subscriptions_not_supported_t = 158,
    subsck_reason_code_subscription_identifiers_not_supported_t = 161,
    subsck_reason_code_wildcard_subscriptions_not_supported_t = 162
};

enum unsuback_reason_code_t {
    unsubsck_reason_code_success_t = 0,
    unsubsck_reason_code_no_subscription_existed_t = 17,
    unsubsck_reason_code_unspecified_error_t = 128,
    unsubsck_reason_code_implementation_specific_error_t = 131,
    unsubsck_reason_code_not_authorized_t = 135,
    unsubsck_reason_code_topic_filter_invalid_t = 143,
    unsubsck_reason_code_packet_identifier_in_use_t = 145
};

enum property_ids_t {
    property_ids_payload_format_indicator_t = 1,
    property_ids_message_expiry_interval_t = 2,
    property_ids_content_type_t = 3,
    property_ids_response_topic_t = 8,
    property_ids_correlation_data_t = 9,
    property_ids_subscription_identifier_t = 11,
    property_ids_session_expiry_interval_t = 17,
    property_ids_assigned_client_identifier_t = 18,
    property_ids_server_keep_alive_t = 19,
    property_ids_authentication_method_t = 21,
    property_ids_authentication_data_t = 22,
    property_ids_request_problem_information_t = 23,
    property_ids_will_delay_interval_t = 24,
    property_ids_request_response_information_t = 25,
    property_ids_response_information_t = 26,
    property_ids_server_reference_t = 28,
    property_ids_reason_string_t = 31,
    property_ids_receive_maximum_t = 33,
    property_ids_topic_alias_maximum_t = 34,
    property_ids_topic_alias_t = 35,
    property_ids_maximum_qos_t = 36,
    property_ids_retain_available_t = 37,
    property_ids_user_property_t = 38,
    property_ids_maximum_packet_size_t = 39,
    property_ids_wildcard_subscription_available_t = 40,
    property_ids_subscription_identifier_available_t = 41,
    property_ids_shared_subscription_available_t = 42
};

/* MQTT 5.0 */
#define PROTOCOL_VERSION 5

#define MAXIMUM_PACKET_SIZE (268435455)  /* (128 ^ 4 - 1) */

struct writer_t {
    struct bitstream_writer_t writer;
    int size;
};

static void writer_init(struct writer_t *self_p, uint8_t *buf_p, int size)
{
    bitstream_writer_init(&self_p->writer, buf_p);
    self_p->size = size;
}

static int writer_written(struct writer_t *self_p)
{
    return (bitstream_writer_size_in_bytes(&self_p->writer));
}

static bool writer_available(struct writer_t *self_p, int size)
{
    return ((self_p->size - writer_written(self_p)) >= size);
}

static void writer_write_u8(struct writer_t *self_p, uint8_t value)
{
    if (writer_available(self_p, 1)) {
        bitstream_writer_write_u8(&self_p->writer, value);
    }
}

static void writer_write_u16(struct writer_t *self_p, uint16_t value)
{
    if (writer_available(self_p, 2)) {
        bitstream_writer_write_u16(&self_p->writer, value);
    }
}

static void writer_write_bytes(struct writer_t *self_p,
                               const uint8_t *buf_p,
                               int size)
{
    if (writer_available(self_p, size)) {
        bitstream_writer_write_bytes(&self_p->writer, buf_p, size);
    }
}

static void writer_write_string(struct writer_t *self_p, const char *string_p)
{
    int length;

    length = strlen(string_p);

    if (writer_available(self_p, length + 2)) {
        bitstream_writer_write_u16(&self_p->writer, length);
        bitstream_writer_write_bytes(&self_p->writer,
                                     (const uint8_t *)string_p,
                                     length);
    }
}

struct reader_t {
    uint8_t *buf_p;
    int size;
    int offset;
};

static void reader_init(struct reader_t *self_p,
                        uint8_t *buf_p,
                        size_t size)
{
    self_p->buf_p = buf_p;
    self_p->size = size;
    self_p->offset = 0;
}

static void reader_get_string(struct reader_t *self_p,
                              char **string_pp,
                              size_t *size_p)
{
    *size_p = ((self_p->buf_p[self_p->offset] << 8)
               | self_p->buf_p[self_p->offset + 1]);
    self_p->offset += 2;
    *string_pp = (char *)&self_p->buf_p[self_p->offset];
    self_p->offset += *size_p;
}

static void reader_get_bytes(struct reader_t *self_p,
                             uint8_t **buf_pp,
                             int *offset_p)
{
    *buf_pp = &self_p->buf_p[self_p->offset];
    *offset_p = self_p->offset;
}

static void pack_variable_integer(struct writer_t *writer_p, int value)
{
    uint8_t encoded_byte;

    if (value == 0) {
        writer_write_u8(writer_p, 0);
    } else {
        while (value > 0) {
            encoded_byte = (value & 0x7f);
            value >>= 7;

            if (value > 0) {
                encoded_byte |= 0x80;
            }

            writer_write_u8(writer_p, encoded_byte);
        }
    }
}

static void pack_fixed_header(struct writer_t *writer_p,
                              uint8_t message_type,
                              uint8_t flags,
                              uint16_t size)
{
    writer_write_u8(writer_p, (message_type << 4) | flags);
    pack_variable_integer(writer_p, size);
}

static size_t pack_connect(struct writer_t *writer_p,
                           const char *client_id_p,
                           int keep_alive_s)
{
    int payload_length;

    payload_length = strlen(client_id_p) + 2;
    pack_fixed_header(writer_p,
                      control_packet_type_connect_t,
                      0,
                      10 + payload_length + 1);
    writer_write_string(writer_p, "MQTT");
    writer_write_u8(writer_p, PROTOCOL_VERSION);
    writer_write_u8(writer_p, 0);
    writer_write_u16(writer_p, keep_alive_s);
    pack_variable_integer(writer_p, 0);
    writer_write_string(writer_p, client_id_p);

    return (writer_written(writer_p));
}

static size_t pack_disconnect(struct writer_t *writer_p,
                              enum disconnect_reason_code_t reason)
{
    pack_fixed_header(writer_p, control_packet_type_disconnect_t, 0, 2);
    writer_write_u8(writer_p, reason);
    pack_variable_integer(writer_p, 0);

    return (writer_written(writer_p));
}

static size_t pack_subscribe(struct writer_t *writer_p,
                             const char *topic_p,
                             uint16_t packet_identifier)
{
    pack_fixed_header(writer_p,
                      control_packet_type_subscribe_t,
                      packet_identifier,
                      strlen(topic_p) + 6);
    writer_write_u16(writer_p, 1);
    writer_write_u8(writer_p, 0);
    writer_write_string(writer_p, topic_p);
    writer_write_u8(writer_p, 0);

    return (writer_written(writer_p));
}

static size_t pack_publish(struct writer_t *writer_p,
                           const char *topic_p,
                           const void *buf_p,
                           size_t size)
{
    pack_fixed_header(writer_p,
                      control_packet_type_publish_t,
                      0,
                      size + strlen(topic_p) + 3);
    writer_write_string(writer_p, topic_p);
    pack_variable_integer(writer_p, 0);
    writer_write_bytes(writer_p, buf_p, size);

    return (writer_written(writer_p));
}

static void unpack_publish(struct asyncio_mqtt_client_t *self_p,
                           char **topic_pp,
                           void *buf_pp,
                           size_t *size_p)
{
    struct reader_t reader;
    size_t topic_size;
    int buf_offset;

    reader_init(&reader, &self_p->packet.buf[0], self_p->packet.size);
    reader_get_string(&reader, topic_pp, &topic_size);
    reader.offset++;
    reader_get_bytes(&reader, buf_pp, &buf_offset);
    (*topic_pp)[topic_size] = '\0';
    *size_p = (self_p->packet.size - buf_offset);
}

static void on_reconnect_timeout(struct asyncio_mqtt_client_t *self_p)
{
    if (!async_timer_is_stopped(&self_p->reconnect_timer)) {
        asyncio_tcp_connect(&self_p->tcp, self_p->host_p, self_p->port);
    }
}

static void start_reconnect_timer(struct asyncio_mqtt_client_t *self_p)
{
    async_timer_start(&self_p->reconnect_timer, 1000);
}

static void stop_reconnect_timer(struct asyncio_mqtt_client_t *self_p)
{
    async_timer_stop(&self_p->reconnect_timer);
}

static void on_tcp_connect_complete(struct asyncio_mqtt_client_t *self_p)
{
    struct writer_t writer;
    uint8_t buf[256];

    if (asyncio_tcp_is_connected(&self_p->tcp)) {
        writer_init(&writer, &buf[0], sizeof(buf));
        asyncio_tcp_write(&self_p->tcp,
                          &buf[0],
                          pack_connect(&writer,
                                       &self_p->client_id[0],
                                       30));
        stop_reconnect_timer(self_p);
    } else {
        start_reconnect_timer(self_p);
    }
}

static void on_tcp_disconnected(struct asyncio_mqtt_client_t *self_p)
{
    self_p->connected = false;
    async_timer_stop(&self_p->keep_alive_timer);
    self_p->on_disconnected(self_p);
    start_reconnect_timer(self_p);
}

static bool read_packet_type(struct asyncio_mqtt_client_t *self_p)
{
    uint8_t ch;
    size_t size;

    size = asyncio_tcp_read(&self_p->tcp, &ch, 1);

    if (size == 1) {
        self_p->packet.type = (ch >> 4);
        self_p->packet.flags = (ch & 0xf);
        self_p->packet.offset = 0;
        self_p->packet.state = packet_state_read_size_t;
    }

    return (false);
}

static bool read_packet_size(struct asyncio_mqtt_client_t *self_p)
{
    size_t size;
    bool complete;

    size = asyncio_tcp_read(&self_p->tcp,
                            &self_p->packet.buf[self_p->packet.offset],
                            1);

    if (size == 0) {
        return (false);
    }

    complete = false;
    self_p->packet.offset++;

    switch (self_p->packet.offset) {

    case 1:
        if ((self_p->packet.buf[0] & 0x80) == 0) {
            self_p->packet.size = self_p->packet.buf[0];
            complete = true;
        }

        break;

    default:
        self_p->packet.state = packet_state_read_type_t;
        break;
    }

    if (complete) {
        if (self_p->packet.size > 0) {
            self_p->packet.state = packet_state_read_data_t;
        } else {
            self_p->packet.state = packet_state_read_type_t;
        }

        self_p->packet.offset = 0;
        complete = (self_p->packet.size == 0);
    }

    return (complete);
}

static bool read_packet_data(struct asyncio_mqtt_client_t *self_p)
{
    size_t size;

    if (self_p->packet.offset == sizeof(self_p->packet.buf)) {
        self_p->packet.state = packet_state_read_type_t;

        return (false);
    }

    size = asyncio_tcp_read(&self_p->tcp,
                            &self_p->packet.buf[self_p->packet.offset],
                            self_p->packet.size - self_p->packet.offset);
    self_p->packet.offset += size;

    if (self_p->packet.offset != self_p->packet.size) {
        return (false);
    }

    self_p->packet.state = packet_state_read_type_t;

    return (true);
}

static bool read_packet(struct asyncio_mqtt_client_t *self_p)
{
    bool complete;

    switch (self_p->packet.state) {

    case packet_state_read_type_t:
        complete = read_packet_type(self_p);
        break;

    case packet_state_read_size_t:
        complete = read_packet_size(self_p);
        break;

    case packet_state_read_data_t:
        complete = read_packet_data(self_p);
        break;

    default:
        complete = false;
        break;
    }

    return (complete);
}

static void handle_connack(struct asyncio_mqtt_client_t *self_p)
{
    self_p->connected = true;
    async_timer_start(&self_p->keep_alive_timer, 1000 * self_p->keep_alive_s);
    self_p->on_connected(self_p->obj_p);
}

static void handle_suback(struct asyncio_mqtt_client_t *self_p)
{
    (void)self_p;
}

static void handle_publish(struct asyncio_mqtt_client_t *self_p)
{
    char *topic_p;
    void *buf_p;
    size_t size;

    unpack_publish(self_p,
                   &topic_p,
                   &buf_p,
                   &size);
    self_p->on_publish(self_p->obj_p, topic_p, buf_p, size);
}

static void handle_pingresp(struct asyncio_mqtt_client_t *self_p)
{
    (void)self_p;
}

static void on_tcp_data(struct asyncio_mqtt_client_t *self_p)
{
    if (!read_packet(self_p)) {
        return;
    }

    switch (self_p->packet.type) {

    case control_packet_type_connack_t:
        handle_connack(self_p);
        break;

    case control_packet_type_suback_t:
        handle_suback(self_p);
        break;

    case control_packet_type_publish_t:
        handle_publish(self_p);
        break;

    case control_packet_type_pingresp_t:
        handle_pingresp(self_p);
        break;

    default:
        break;
    }
}

static uint16_t next_packet_identifier(struct asyncio_mqtt_client_t *self_p)
{
    uint16_t packet_identifier;

    packet_identifier = self_p->next_packet_identifier;
    self_p->next_packet_identifier++;

    if (self_p->next_packet_identifier == 0) {
        self_p->next_packet_identifier = 1;
    }

    return (packet_identifier);
}

static size_t pack_pingreq(struct writer_t *writer_p)
{
    pack_fixed_header(writer_p, control_packet_type_pingreq_t, 0, 0);

    return (writer_written(writer_p));
}

static void on_keep_alive_timeout(struct asyncio_mqtt_client_t *self_p)
{
    struct writer_t writer;
    uint8_t buf[8];

    writer_init(&writer, &buf[0], sizeof(buf));
    asyncio_tcp_write(&self_p->tcp, &buf[0], pack_pingreq(&writer));
}

void asyncio_mqtt_client_init(struct asyncio_mqtt_client_t *self_p,
                              const char *host_p,
                              int port,
                              async_func_t on_connected,
                              async_func_t on_disconnected,
                              asyncio_mqtt_client_on_publish_t on_publish,
                              void *obj_p,
                              struct asyncio_t *asyncio_p)
{
    self_p->host_p = host_p;
    self_p->port = port;
    self_p->on_connected = on_connected;
    self_p->on_disconnected = on_disconnected;
    self_p->on_publish = on_publish;
    self_p->obj_p = obj_p;
    self_p->asyncio_p = asyncio_p;
    sprintf(&self_p->client_id[0], "async-12345");
    self_p->keep_alive_s = 10;
    self_p->response_timeout = 5;
    self_p->session_expiry_interval = 0;
    self_p->connected = false;
    self_p->next_packet_identifier = 1;
    asyncio_tcp_init(&self_p->tcp,
                     (async_func_t)on_tcp_connect_complete,
                     (async_func_t)on_tcp_disconnected,
                     (async_func_t)on_tcp_data,
                     self_p,
                     asyncio_p);
    self_p->packet.state = packet_state_read_type_t;
    async_timer_init(&self_p->keep_alive_timer,
                     (async_func_t)on_keep_alive_timeout,
                     self_p,
                     ASYNC_TIMER_PERIODIC,
                     &asyncio_p->async);
    async_timer_init(&self_p->reconnect_timer,
                     (async_func_t)on_reconnect_timeout,
                     self_p,
                     0,
                     &asyncio_p->async);
}

void asyncio_mqtt_client_set_client_id(struct asyncio_mqtt_client_t *self_p,
                                       const char *client_id_p)
{
    strncpy(&self_p->client_id[0], client_id_p, sizeof(self_p->client_id));
}

void asyncio_mqtt_client_set_response_timeout(struct asyncio_mqtt_client_t *self_p,
                                              int response_timeout)
{
    self_p->response_timeout = response_timeout;
}

void asyncio_mqtt_client_set_session_expiry_interval(
    struct asyncio_mqtt_client_t *self_p,
    int session_expiry_interval)
{
    self_p->session_expiry_interval = session_expiry_interval;
}

void asyncio_mqtt_client_start(struct asyncio_mqtt_client_t *self_p)
{
    asyncio_tcp_connect(&self_p->tcp, self_p->host_p, self_p->port);
}

void asyncio_mqtt_client_stop(struct asyncio_mqtt_client_t *self_p)
{
    struct writer_t writer;
    uint8_t buf[8];

    writer_init(&writer, &buf[0], sizeof(buf));
    asyncio_tcp_write(
        &self_p->tcp,
        &buf[0],
        pack_disconnect(&writer,
                        disconnect_reason_code_normal_disconnection_t));
    asyncio_tcp_disconnect(&self_p->tcp);
    self_p->connected = false;
    async_timer_stop(&self_p->keep_alive_timer);
}

void asyncio_mqtt_client_subscribe(struct asyncio_mqtt_client_t *self_p,
                                   const char *topic_p)
{
    struct writer_t writer;
    uint8_t buf[512];

    writer_init(&writer, &buf[0], sizeof(buf));
    asyncio_tcp_write(&self_p->tcp,
                      &buf[0],
                      pack_subscribe(&writer,
                                     topic_p,
                                     next_packet_identifier(self_p)));
}

void asyncio_mqtt_client_unsubscribe(struct asyncio_mqtt_client_t *self_p,
                                     const char *topic_p)
{
    (void)self_p;
    (void)topic_p;
}

void asyncio_mqtt_client_publish(struct asyncio_mqtt_client_t *self_p,
                                 const char *topic_p,
                                 const void *buf_p,
                                 size_t size)
{
    struct writer_t writer;
    uint8_t buf[512];

    writer_init(&writer, &buf[0], sizeof(buf));
    asyncio_tcp_write(&self_p->tcp,
                      &buf[0],
                      pack_publish(&writer, topic_p, buf_p, size));
}
