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
