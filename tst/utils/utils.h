#ifndef UTILS_H
#define UTILS_H

#include "async.h"

void channel_open(struct async_channel_t *self_p);

void channel_close(struct async_channel_t *self_p);

size_t channel_read(struct async_channel_t *self_p,
                    void *buf_p,
                    size_t size);

void channel_write(struct async_channel_t *self_p,
                   const void *buf_p,
                   size_t size);

void mqtt_on_connected(void *obj_p);

void mqtt_on_disconnected(void *obj_p);

void mqtt_on_subscribe_complete(void *obj_p, uint16_t transaction_id);

void mqtt_on_publish(void *obj_p,
                     const char *topic_p,
                     const uint8_t *buf_p,
                     size_t size);

#endif
