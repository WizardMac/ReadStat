#include <stdlib.h>

#include "../readstat.h"

#include "test_types.h"
#include "test_buffer.h"

rt_buffer_t *buffer_init() {
    rt_buffer_t *buffer = calloc(1, sizeof(rt_buffer_t));
    buffer->size = 1024;
    buffer->bytes = malloc(buffer->size);

    return buffer;
}

void buffer_reset(rt_buffer_t *buffer) {
    buffer->used = 0;
}

void buffer_free(rt_buffer_t *buffer) {
    free(buffer->bytes);
    free(buffer);
}
