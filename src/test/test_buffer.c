#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_buffer.h"

rt_buffer_t *buffer_init(void) {
    rt_buffer_t *buffer = calloc(1, sizeof(rt_buffer_t));
    if (!buffer) {
        return NULL;
    }
    buffer->size = 1024;
    buffer->bytes = malloc(buffer->size);
    if (!buffer->bytes) {
        free(buffer);
        return NULL;
    }
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

readstat_error_t buffer_read_from_resource(rt_buffer_t *buffer, char *resource_name) {
    const char *path_prefix = "resources/";
    int retval = READSTAT_OK;

    FILE* file = NULL;
    char *resource_path = malloc(strlen(path_prefix) + strlen(resource_name) + 1);
    if (resource_path == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    strcpy(resource_path, path_prefix);
    strcat(resource_path, resource_name);
    file = fopen(resource_path, "rb");
    if (!file) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    const size_t CHUNK_SIZE = 1024;
    while (1) {
        buffer_grow(buffer, CHUNK_SIZE);
        if (!buffer->bytes) {
            free(buffer);
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }

        size_t bytes_read = fread(buffer->bytes + buffer->used, 1, CHUNK_SIZE, file);
        buffer->used += bytes_read;
        if (ferror(file)) {
            buffer_free(buffer);
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if (bytes_read < CHUNK_SIZE) {
            break;
        }
    }

cleanup:
    free(resource_path);
    if (file) {
        fclose(file);
    }
    return retval;
}
