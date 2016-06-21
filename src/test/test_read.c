#include <stdlib.h>

#include "../readstat.h"

#include "test_types.h"
#include "test_error.h"
#include "test_buffer.h"
#include "test_readstat.h"
#include "test_read.h"

static rt_buffer_ctx_t *buffer_ctx_init(rt_buffer_t *buffer) {
    rt_buffer_ctx_t *buffer_ctx = calloc(1, sizeof(rt_buffer_ctx_t));
    buffer_ctx->buffer = buffer;
    return buffer_ctx;
}

static void buffer_ctx_reset(rt_buffer_ctx_t *buffer_ctx) {
    buffer_reset(buffer_ctx->buffer);
    buffer_ctx->pos = 0;
}

rt_parse_ctx_t *parse_ctx_init(rt_buffer_t *buffer, rt_test_file_t *file) {
    rt_parse_ctx_t *parse_ctx = calloc(1, sizeof(rt_parse_ctx_t));
    parse_ctx->buffer_ctx = buffer_ctx_init(buffer);
    parse_ctx->file = file;
    return parse_ctx;
}

void parse_ctx_reset(rt_parse_ctx_t *parse_ctx, long file_format) {
    parse_ctx->file_format = file_format;
    parse_ctx->var_index = -1;
    parse_ctx->obs_index = -1;
    buffer_ctx_reset(parse_ctx->buffer_ctx);
}

void parse_ctx_free(rt_parse_ctx_t *parse_ctx) {
    if (parse_ctx->buffer_ctx) {
        free(parse_ctx->buffer_ctx);
    }
    free(parse_ctx);
}

static int rt_open_handler(const char *path, void *io_ctx) {
    return 0;
}

static int rt_close_handler(void *io_ctx) {
    return 0;
}

static readstat_off_t rt_seek_handler(readstat_off_t offset,
        readstat_io_flags_t whence, void *io_ctx) {
    rt_buffer_ctx_t *buffer_ctx = (rt_buffer_ctx_t *)io_ctx;
    readstat_off_t newpos = -1;
    if (whence == READSTAT_SEEK_SET) {
        newpos = offset;
    } else if (whence == READSTAT_SEEK_CUR) {
        newpos = buffer_ctx->pos + offset;
    } else if (whence == READSTAT_SEEK_END) {
        newpos = buffer_ctx->buffer->used + offset;
    }

    if (newpos < 0)
        return -1;

    if (newpos > buffer_ctx->buffer->used)
        return -1;

    buffer_ctx->pos = newpos;
    return newpos;
}

static ssize_t rt_read_handler(void *buf, size_t nbytes, void *io_ctx) {
    rt_buffer_ctx_t *buffer_ctx = (rt_buffer_ctx_t *)io_ctx;
    ssize_t bytes_copied = -1;
    ssize_t bytes_left = buffer_ctx->buffer->used - buffer_ctx->pos;
    if (nbytes <= bytes_left) {
        memcpy(buf, buffer_ctx->buffer->bytes + buffer_ctx->pos, nbytes);
        bytes_copied = nbytes;
    } else if (bytes_left > 0) {
        memcpy(buf, buffer_ctx->buffer->bytes + buffer_ctx->pos, bytes_left);
        bytes_copied = bytes_left;
    }
    buffer_ctx->pos += bytes_copied;
    return bytes_copied;
}

static readstat_error_t rt_update_handler(long file_size,
        readstat_progress_handler progress_handler, void *user_ctx,
        void *io_ctx) {
    if (!progress_handler)
        return READSTAT_OK;

    rt_buffer_ctx_t *buffer_ctx = (rt_buffer_ctx_t *)io_ctx;

    if (progress_handler(1.0 * buffer_ctx->pos / buffer_ctx->buffer->used, user_ctx))
        return READSTAT_ERROR_USER_ABORT;

    return READSTAT_OK;
}

static int handle_info(int obs_count, int var_count, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;

    rt_ctx->var_index = -1;
    rt_ctx->obs_index = -1;

    push_error_if_doubles_differ(rt_ctx, 
            rt_ctx->file->columns_count, var_count, 
            "Number of variables");

    push_error_if_doubles_differ(rt_ctx, 
            rt_ctx->file->rows, obs_count, 
            "Number of observations");

    return 0;
}

static int handle_variable(int index, readstat_variable_t *variable,
                           const char *val_labels, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;
    rt_column_t *column = &rt_ctx->file->columns[index];

    rt_ctx->var_index = index;

    push_error_if_strings_differ(rt_ctx, column->name, 
            readstat_variable_get_name(variable),
            "Column names");

    push_error_if_strings_differ(rt_ctx, column->label,
            readstat_variable_get_label(variable),
            "Column labels");

    return 0;
}

static int handle_value(int obs_index, int var_index, readstat_value_t value, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;
    rt_column_t *column = &rt_ctx->file->columns[var_index];

    rt_ctx->obs_index = obs_index;
    rt_ctx->var_index = var_index;

    push_error_if_values_differ(rt_ctx, 
            column->values[obs_index],
            value, "Data values");

    return 0;
}

readstat_error_t read_file(rt_parse_ctx_t *parse_ctx, long format) {
    readstat_error_t error = READSTAT_OK;

    readstat_parser_t *parser = readstat_parser_init();

    readstat_set_open_handler(parser, rt_open_handler);
    readstat_set_close_handler(parser, rt_close_handler);
    readstat_set_seek_handler(parser, rt_seek_handler);
    readstat_set_read_handler(parser, rt_read_handler);
    readstat_set_update_handler(parser, rt_update_handler);
    readstat_set_io_ctx(parser, parse_ctx->buffer_ctx);

    readstat_set_info_handler(parser, &handle_info);
    readstat_set_variable_handler(parser, &handle_variable);
    readstat_set_value_handler(parser, &handle_value);

    if ((format & RT_FORMAT_DTA)) {
        error = readstat_parse_dta(parser, NULL, parse_ctx);
    } else if (format == RT_FORMAT_SAV) {
        error = readstat_parse_sav(parser, NULL, parse_ctx);
    }
    if (error != READSTAT_OK)
        goto cleanup;

cleanup:
    readstat_parser_free(parser);

    return error;
}

