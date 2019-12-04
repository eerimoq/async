#include "nala.h"
#include "async.h"

TEST(test_init_all_null)
{
    struct async_t async;
    struct async_channel_t channel;
    char ch;

    async_init(&async);
    async_channel_init(&channel, NULL, NULL, NULL, NULL, &async);
    async_channel_open(&channel);
    async_channel_read(&channel, &ch, 1);
    ch = 1;
    async_channel_write(&channel, &ch, 1);
    async_channel_close(&channel);
    async_channel_opened(&channel, 0);
    async_channel_closed(&channel);
    async_channel_input(&channel);
    async_process(&async);
}

TEST(test_set_on_null)
{
    struct async_t async;
    struct async_channel_t channel;
    char ch;

    async_init(&async);
    async_channel_init(&channel, NULL, NULL, NULL, NULL, &async);
    async_channel_set_on(&channel, NULL, NULL, NULL, NULL);
    async_channel_open(&channel);
    async_channel_read(&channel, &ch, 1);
    ch = 1;
    async_channel_write(&channel, &ch, 1);
    async_channel_close(&channel);
    async_channel_opened(&channel, 0);
    async_channel_closed(&channel);
    async_channel_input(&channel);
    async_process(&async);
}
