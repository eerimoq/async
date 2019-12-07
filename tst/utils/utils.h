#ifndef UTILS_H
#define UTILS_H

#include "async.h"

void channel_open(struct async_channel_t *self_p);

void channel_close(struct async_channel_t *self_p);

ssize_t channel_read(struct async_channel_t *self_p,
                     void *buf_p,
                     size_t size);

ssize_t channel_write(struct async_channel_t *self_p,
                      const void *buf_p,
                      size_t size);

#endif
