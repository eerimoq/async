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
#include "async.h"
#include "async/modules/mqtt_client.h"
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

static void writer_write_binary(struct writer_t *self_p,
                                const uint8_t *buf_p,
                                size_t size)
{
    if (writer_available(self_p, size + 2)) {
        bitstream_writer_write_u16(&self_p->writer, size);
        bitstream_writer_write_bytes(&self_p->writer, buf_p, size);
    }
}

static void writer_write_string(struct writer_t *self_p, const char *string_p)
{
    writer_write_binary(self_p,
                        (const uint8_t *)string_p,
                        strlen(string_p));
}

struct reader_t {
    struct bitstream_reader_t reader;
    uint8_t *buf_p;
    int size;
};

static void reader_init(struct reader_t *self_p,
                        uint8_t *buf_p,
                        size_t size)
{
    bitstream_reader_init(&self_p->reader, buf_p);
    self_p->buf_p = buf_p;
    self_p->size = size;
}

static uint16_t reader_offset(struct reader_t *self_p)
{
    return (bitstream_reader_tell(&self_p->reader) / 8);
}

static bool reader_available(struct reader_t *self_p, int size)
{
    bool ok;

    ok = ((reader_offset(self_p) + size) <= self_p->size);

    if (!ok) {
        self_p->size = -1;
    }

    return (ok);
}

static bool reader_ok(struct reader_t *self_p)
{
    return (self_p->size >= 0);
}

static void reader_seek(struct reader_t *self_p,
                        int offset)
{
    if (reader_available(self_p, offset)) {
        bitstream_reader_seek(&self_p->reader, 8 * offset);
    }
}

static uint16_t reader_read_u16(struct reader_t *self_p)
{
    uint16_t value;

    if (reader_available(self_p, 2)) {
        value = bitstream_reader_read_u16(&self_p->reader);
    } else {
        value = 0;
    }

    return (value);
}

static void reader_get_string(struct reader_t *self_p,
                              char **string_pp,
                              size_t *size_p)
{
    *size_p = reader_read_u16(self_p);

    if (reader_available(self_p, *size_p)) {
        *string_pp = (char *)&self_p->buf_p[reader_offset(self_p)];
        reader_seek(self_p, *size_p);
    }
}

static void reader_null_terminate_string(struct reader_t *self_p,
                                         char *string_p,
                                         size_t size)
{
    if (reader_ok(self_p)) {
        string_p[size] = '\0';
    }
}

static uint8_t *reader_pointer(struct reader_t *self_p)
{
    return (&self_p->buf_p[reader_offset(self_p)]);
}

static void on_subscribe_complete_null(void *obj_p,
                                       uint16_t transaction_id)
{
    (void)obj_p;
    (void)transaction_id;
}

static void on_unsubscribe_complete_null(void *obj_p,
                                         uint16_t transaction_id)
{
    (void)obj_p;
    (void)transaction_id;
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
                           struct async_mqtt_client_will_t *will_p,
                           int keep_alive_s)
{
    uint8_t flags;
    int payload_length;

    flags = CLEAN_START;
    payload_length = strlen(client_id_p) + 2;

    if (will_p->topic_p != NULL) {
        flags |= WILL_FLAG;
        payload_length++;
        payload_length += (strlen(will_p->topic_p) + 2);
        payload_length += (will_p->message.size + 2);
    }

    pack_fixed_header(writer_p,
                      control_packet_type_connect_t,
                      0,
                      10 + payload_length + 1);
    writer_write_string(writer_p, "MQTT");
    writer_write_u8(writer_p, PROTOCOL_VERSION);
    writer_write_u8(writer_p, flags);
    writer_write_u16(writer_p, keep_alive_s);
    pack_variable_integer(writer_p, 0);
    writer_write_string(writer_p, client_id_p);

    if (flags & WILL_FLAG) {
        pack_variable_integer(writer_p, 0);
        writer_write_string(writer_p, will_p->topic_p);
        writer_write_binary(writer_p,
                            will_p->message.buf_p,
                            will_p->message.size);
    }

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
                      0,
                      strlen(topic_p) + 6);
    writer_write_u16(writer_p, packet_identifier);
    writer_write_u8(writer_p, 0);
    writer_write_string(writer_p, topic_p);
    writer_write_u8(writer_p, 0);

    return (writer_written(writer_p));
}

static bool unpack_suback(struct async_mqtt_client_t *self_p,
                          uint16_t *packet_identifier_p)
{
    struct reader_t reader;

    reader_init(&reader, &self_p->packet.buf[0], self_p->packet.size);
    *packet_identifier_p = reader_read_u16(&reader);

    return (reader_ok(&reader));
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

static bool unpack_publish(struct async_mqtt_client_t *self_p,
                           char **topic_pp,
                           uint8_t **buf_pp,
                           size_t *size_p)
{
    struct reader_t reader;
    size_t topic_size;

    reader_init(&reader, &self_p->packet.buf[0], self_p->packet.size);
    reader_get_string(&reader, topic_pp, &topic_size);
    reader_seek(&reader, 1);
    reader_null_terminate_string(&reader, *topic_pp, topic_size);
    *buf_pp = reader_pointer(&reader);
    *size_p = (self_p->packet.size - reader_offset(&reader));

    return (reader_ok(&reader));
}

static void on_reconnect_timeout(struct async_timer_t *timer_p)
{
    struct async_mqtt_client_t *self_p;

    self_p = async_container_of(timer_p, typeof(*self_p), reconnect_timer);
    async_tcp_client_connect(&self_p->tcp, self_p->host_p, self_p->port);
}

static void start_reconnect_timer(struct async_mqtt_client_t *self_p)
{
    async_timer_start(&self_p->reconnect_timer);
}

static void stop_reconnect_timer(struct async_mqtt_client_t *self_p)
{
    async_timer_stop(&self_p->reconnect_timer);
}

static void on_tcp_connected(struct async_tcp_client_t *tcp_p, int res)
{
    struct writer_t writer;
    uint8_t buf[256];
    struct async_mqtt_client_t *self_p;

    self_p = async_container_of(tcp_p, typeof(*self_p), tcp);

    if (res == 0) {
        writer_init(&writer, &buf[0], sizeof(buf));
        async_tcp_client_write(&self_p->tcp,
                               &buf[0],
                               pack_connect(&writer,
                                            &self_p->client_id[0],
                                            &self_p->will,
                                            30));
        stop_reconnect_timer(self_p);
    } else {
        start_reconnect_timer(self_p);
    }
}

static void on_tcp_disconnected(struct async_tcp_client_t *tcp_p)
{
    struct async_mqtt_client_t *self_p;

    self_p = async_container_of(tcp_p, typeof(*self_p), tcp);
    self_p->connected = false;
    async_timer_stop(&self_p->keep_alive_timer);
    self_p->on_disconnected(self_p);
    start_reconnect_timer(self_p);
}

static bool read_packet_type(struct async_mqtt_client_t *self_p)
{
    uint8_t ch;
    size_t size;

    size = async_tcp_client_read(&self_p->tcp, &ch, 1);

    if (size == 1) {
        self_p->packet.type = (ch >> 4);
        self_p->packet.flags = (ch & 0xf);
        self_p->packet.offset = 0;
        self_p->packet.state = packet_state_read_size_t;
    }

    return (false);
}

static bool read_packet_size(struct async_mqtt_client_t *self_p)
{
    size_t size;
    bool complete;

    size = async_tcp_client_read(&self_p->tcp,
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

    case 2:
        if ((self_p->packet.buf[1] & 0x80) == 0) {
            self_p->packet.size = self_p->packet.buf[1];
            self_p->packet.size *= 128;
            self_p->packet.size += (self_p->packet.buf[0] & 0x7f);
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

static bool read_packet_data(struct async_mqtt_client_t *self_p)
{
    size_t size;

    if (self_p->packet.offset == sizeof(self_p->packet.buf)) {
        self_p->packet.state = packet_state_read_type_t;

        return (false);
    }

    size = async_tcp_client_read(&self_p->tcp,
                                 &self_p->packet.buf[self_p->packet.offset],
                                 self_p->packet.size - self_p->packet.offset);
    self_p->packet.offset += size;

    if (self_p->packet.offset != self_p->packet.size) {
        return (false);
    }

    self_p->packet.state = packet_state_read_type_t;

    return (true);
}

static bool read_packet(struct async_mqtt_client_t *self_p)
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

static void handle_connack(struct async_mqtt_client_t *self_p)
{
    self_p->connected = true;
    async_timer_start(&self_p->keep_alive_timer);
    self_p->on_connected(self_p->obj_p);
}

static void handle_suback(struct async_mqtt_client_t *self_p)
{
    uint16_t packet_identifier;

    if (unpack_suback(self_p, &packet_identifier)) {
        self_p->on_subscribe_complete(self_p->obj_p, packet_identifier);
    }
}

static void handle_publish(struct async_mqtt_client_t *self_p)
{
    char *topic_p;
    uint8_t *buf_p;
    size_t size;

    if (unpack_publish(self_p, &topic_p, &buf_p, &size)) {
        self_p->on_publish(self_p->obj_p, topic_p, buf_p, size);
    }
}

static void handle_pingresp(struct async_mqtt_client_t *self_p)
{
    async_timer_start(&self_p->keep_alive_timer);
}

static void on_tcp_input(struct async_tcp_client_t *tcp_p)
{
    struct async_mqtt_client_t *self_p;

    self_p = async_container_of(tcp_p, typeof(*self_p), tcp);

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

static uint16_t next_packet_identifier(struct async_mqtt_client_t *self_p)
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

static void on_keep_alive_timeout(struct async_timer_t *timer_p)
{
    struct writer_t writer;
    uint8_t buf[8];
    struct async_mqtt_client_t *self_p;

    self_p = async_container_of(timer_p, typeof(*self_p), keep_alive_timer);
    writer_init(&writer, &buf[0], sizeof(buf));
    async_tcp_client_write(&self_p->tcp, &buf[0], pack_pingreq(&writer));
}

void async_mqtt_client_init(struct async_mqtt_client_t *self_p,
                            const char *host_p,
                            int port,
                            async_func_t on_connected,
                            async_func_t on_disconnected,
                            async_mqtt_client_on_publish_t on_publish,
                            void *obj_p,
                            struct async_t *async_p)
{
    self_p->host_p = host_p;
    self_p->port = port;
    self_p->on_connected = on_connected;
    self_p->on_disconnected = on_disconnected;
    self_p->on_publish = on_publish;
    self_p->on_subscribe_complete = on_subscribe_complete_null;
    self_p->on_unsubscribe_complete = on_unsubscribe_complete_null;
    self_p->obj_p = obj_p;
    self_p->async_p = async_p;
    sprintf(&self_p->client_id[0], "async-12345");
    self_p->keep_alive_s = 10;
    self_p->response_timeout = 5;
    self_p->session_expiry_interval = 0;
    self_p->will.topic_p = NULL;
    self_p->connected = false;
    self_p->next_packet_identifier = 1;
    async_tcp_client_init(&self_p->tcp,
                          on_tcp_connected,
                          on_tcp_disconnected,
                          on_tcp_input,
                          async_p);
    self_p->packet.state = packet_state_read_type_t;
    async_timer_init(&self_p->keep_alive_timer,
                     on_keep_alive_timeout,
                     1000 * self_p->keep_alive_s,
                     0,
                     async_p);
    async_timer_init(&self_p->reconnect_timer,
                     on_reconnect_timeout,
                     1000,
                     0,
                     async_p);
}

void async_mqtt_client_set_client_id(struct async_mqtt_client_t *self_p,
                                     const char *client_id_p)
{
    strncpy(&self_p->client_id[0], client_id_p, sizeof(self_p->client_id));
}

void async_mqtt_client_set_response_timeout(struct async_mqtt_client_t *self_p,
                                            int response_timeout)
{
    self_p->response_timeout = response_timeout;
}

void async_mqtt_client_set_session_expiry_interval(
    struct async_mqtt_client_t *self_p,
    int session_expiry_interval)
{
    self_p->session_expiry_interval = session_expiry_interval;
}

void async_mqtt_client_set_will(struct async_mqtt_client_t *self_p,
                                const char *topic_p,
                                uint8_t *buf_p,
                                size_t size)
{
    self_p->will.topic_p = topic_p;
    self_p->will.message.buf_p = buf_p;
    self_p->will.message.size = size;
}

void async_mqtt_client_set_on_subscribe_complete(
    struct async_mqtt_client_t *self_p,
    async_mqtt_client_on_subscribe_complete_t on_subscribe_complete)
{
    self_p->on_subscribe_complete = on_subscribe_complete;
}

void async_mqtt_client_set_on_unsubscribe_complete(
    struct async_mqtt_client_t *self_p,
    async_mqtt_client_on_unsubscribe_complete_t on_unsubscribe_complete)
{
    self_p->on_unsubscribe_complete = on_unsubscribe_complete;
}

void async_mqtt_client_start(struct async_mqtt_client_t *self_p)
{
    async_tcp_client_connect(&self_p->tcp, self_p->host_p, self_p->port);
}

void async_mqtt_client_stop(struct async_mqtt_client_t *self_p)
{
    struct writer_t writer;
    uint8_t buf[8];

    writer_init(&writer, &buf[0], sizeof(buf));
    async_tcp_client_write(
        &self_p->tcp,
        &buf[0],
        pack_disconnect(&writer,
                        disconnect_reason_code_normal_disconnection_t));
    async_tcp_client_disconnect(&self_p->tcp);
    self_p->connected = false;
    async_timer_stop(&self_p->keep_alive_timer);
}

uint16_t async_mqtt_client_subscribe(struct async_mqtt_client_t *self_p,
                                     const char *topic_p)
{
    struct writer_t writer;
    uint8_t buf[512];
    uint16_t packet_identifier;

    writer_init(&writer, &buf[0], sizeof(buf));
    packet_identifier = next_packet_identifier(self_p);
    async_tcp_client_write(&self_p->tcp,
                           &buf[0],
                           pack_subscribe(&writer,
                                          topic_p,
                                          packet_identifier));

    return (packet_identifier);
}

uint16_t async_mqtt_client_unsubscribe(struct async_mqtt_client_t *self_p,
                                       const char *topic_p)
{
    (void)self_p;
    (void)topic_p;

    return (0);
}

void async_mqtt_client_publish(struct async_mqtt_client_t *self_p,
                               const char *topic_p,
                               const void *buf_p,
                               size_t size)
{
    struct writer_t writer;
    uint8_t buf[512];

    writer_init(&writer, &buf[0], sizeof(buf));
    async_tcp_client_write(&self_p->tcp,
                           &buf[0],
                           pack_publish(&writer, topic_p, buf_p, size));
}
