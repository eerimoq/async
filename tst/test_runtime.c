#include <sys/eventfd.h>
#include "nala.h"
#include "async.h"

TEST(create)
{
    struct async_runtime_t *runtime_p;

    runtime_p = async_runtime_create();
    ASSERT_NE(runtime_p, NULL);
}
