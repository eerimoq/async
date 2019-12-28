#include <time.h>
#include "utils.h"
#include "nala.h"
#include "nala_mocks.h"

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

void mqtt_on_disconnected(void *obj_p)
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

void mock_prepare_time_ms(int now_ms)
{
    struct timespec timespec;

    clock_gettime_mock_once(CLOCK_MONOTONIC, 0);
    timespec.tv_sec = now_ms / 1000;
    timespec.tv_nsec = (now_ms % 1000) * 1000000;
    clock_gettime_mock_set___tp_out(&timespec, sizeof(timespec));
}

void mock_prepare_process(int evaluate_ms, int next_ms)
{
    mock_prepare_time_ms(evaluate_ms);

    if (next_ms != -1) {
        mock_prepare_time_ms(next_ms);
    }
}
