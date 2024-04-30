#include <stdlib.h>

#include "test_buffer.h"

rt_buffer_t *buffer_init(void) {
    rt_buffer_t *buffer = calloc(1, sizeof(rt_buffer_t));
    buffer->size = 1024;
    buffer->bytes = malloc(buffer->size);

    return buffer;
}

void buffer_reset(rt_buffer_t *buffer) {
    buffer->used = 0;
}

void buffer_grow(rt_buffer_t *buffer, size_t len) {
    while (len > buffer->size - buffer->used) {
        buffer->size *= 2;
    }
    buffer->bytes = realloc(buffer->bytes, buffer->size);
}

void buffer_free(rt_buffer_t *buffer) {
    free(buffer->bytes);
    free(buffer);
}

rt_buffer_ctx_t *buffer_ctx_init(rt_buffer_t *buffer) {
    rt_buffer_ctx_t *buffer_ctx = calloc(1, sizeof(rt_buffer_ctx_t));
    buffer_ctx->buffer = buffer;
    return buffer_ctx;
}

void buffer_ctx_reset(rt_buffer_ctx_t *buffer_ctx) {
    buffer_reset(buffer_ctx->buffer);
    buffer_ctx->pos = 0;
}
