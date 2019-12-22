#include "utils.h"
#include "nala.h"

void channel_open(struct async_channel_t *self_p)
{
    (void)self_p;

    FAIL();
}

void channel_close(struct async_channel_t *self_p)
{
    (void)self_p;

    FAIL();
}

ssize_t channel_read(struct async_channel_t *self_p,
                     void *buf_p,
                     size_t size)
{
    (void)self_p;
    (void)buf_p;

    FAIL();

    return (size);
}

ssize_t channel_write(struct async_channel_t *self_p,
                      const void *buf_p,
                      size_t size)
{
    (void)self_p;
    (void)buf_p;

    FAIL();

    return (size);
}

void mqtt_on_connected(void *obj_p)
{
    (void)obj_p;

    FAIL();
}

void mqtt_on_subscribe_complete(void *obj_p, uint16_t transaction_id)
{
    (void)obj_p;
    (void)transaction_id;

    FAIL();
}

void mqtt_on_publish(void *obj_p,
                     const char *topic_p,
                     const uint8_t *buf_p,
                     size_t size)
{
    (void)obj_p;
    (void)topic_p;
    (void)buf_p;
    (void)size;

    FAIL();
}
