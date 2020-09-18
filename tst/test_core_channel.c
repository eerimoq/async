#include "nala.h"
#include "async.h"

TEST(init_use_default_callbacks)
{
    struct async_t async;
    struct async_channel_t channel;
    char ch;

    async_init(&async);
    async_channel_init(&channel, NULL, NULL, NULL, NULL, NULL, NULL, &async);
    ASSERT_EQ(async_channel_open(&channel), 0);
    ASSERT_EQ(async_channel_read(&channel, &ch, 1), 1);
    ch = 1;
    async_channel_write(&channel, &ch, 1);
    async_channel_close(&channel);
    async_channel_closed(&channel);
    async_channel_input(&channel);
    ASSERT_EQ(async_channel_readable_size(&channel), 0);
    ASSERT_EQ(async_channel_writable_size(&channel), 0);
    async_process(&async);
}

int init_use_non_default_callbacks_open(struct async_channel_t *self_p)
{
    (void)self_p;

    return (0);
}

void init_use_non_default_callbacks_close(struct async_channel_t *self_p)
{
    (void)self_p;
}

size_t init_use_non_default_callbacks_read(struct async_channel_t *self_p,
                                           void *buf_p,
                                           size_t size)
{
    (void)self_p;
    (void)buf_p;

    return (size);
}

void init_use_non_default_callbacks_write(struct async_channel_t *self_p,
                                          const void *buf_p,
                                          size_t size)
{
    (void)self_p;
    (void)buf_p;
    (void)size;
}

size_t init_use_non_default_callbacks_readable_size(struct async_channel_t *self_p)
{
    (void)self_p;

    return (0);
}

size_t init_use_non_default_callbacks_writable_size(struct async_channel_t *self_p)
{
    (void)self_p;

    return (0);
}

TEST(init_use_non_default_callbacks)
{
    struct async_t async;
    struct async_channel_t channel;
    char ch;

    async_init(&async);
    async_channel_init(&channel,
                       init_use_non_default_callbacks_open,
                       init_use_non_default_callbacks_close,
                       init_use_non_default_callbacks_read,
                       init_use_non_default_callbacks_write,
                       init_use_non_default_callbacks_readable_size,
                       init_use_non_default_callbacks_writable_size,
                       &async);
    init_use_non_default_callbacks_open_mock_once(5);
    ASSERT_EQ(async_channel_open(&channel), 5);
    init_use_non_default_callbacks_read_mock_once(1, 6);
    ASSERT_EQ(async_channel_read(&channel, &ch, 1), 6);
    ch = 1;
    init_use_non_default_callbacks_write_mock_once(1);
    async_channel_write(&channel, &ch, 1);
    init_use_non_default_callbacks_close_mock_once();
    async_channel_close(&channel);
    async_channel_closed(&channel);
    init_use_non_default_callbacks_readable_size_mock_once(7);
    ASSERT_EQ(async_channel_readable_size(&channel), 7);
    init_use_non_default_callbacks_writable_size_mock_once(8);
    ASSERT_EQ(async_channel_writable_size(&channel), 8);
    async_process(&async);
}

TEST(set_on_null)
{
    struct async_t async;
    struct async_channel_t channel;
    char ch;

    async_init(&async);
    async_channel_init(&channel, NULL, NULL, NULL, NULL, NULL, NULL, &async);
    async_channel_set_on(&channel, NULL, NULL, NULL);
    ASSERT_EQ(async_channel_open(&channel), 0);
    ASSERT_EQ(async_channel_read(&channel, &ch, 1), 1);
    ch = 1;
    async_channel_write(&channel, &ch, 1);
    async_channel_close(&channel);
    async_channel_closed(&channel);
    async_channel_input(&channel);
    ASSERT_EQ(async_channel_readable_size(&channel), 0);
    ASSERT_EQ(async_channel_writable_size(&channel), 0);
    async_process(&async);
}
