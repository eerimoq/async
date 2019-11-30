#ifndef ASYNC_LINUX_INTERNAL_H
#define ASYNC_LINUX_INTERNAL_H

#include "async.h"

void async_tcp_client_connect_write(struct async_tcp_client_t *self_p,
                                    const char *host_p,
                                    int port);

void async_tcp_client_disconnect_write(struct async_tcp_client_t *self_p);

void async_tcp_client_data_complete_write(struct async_tcp_client_t *self_p,
                                          bool closed);

#endif
