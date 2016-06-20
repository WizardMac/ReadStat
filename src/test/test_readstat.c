
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "../readstat.h"

#define MAX_ROWS     10
#define MAX_STRING   64

typedef enum rt_file_format_e {
    RT_FORMAT_DTA,
    RT_FORMAT_SAV
} rt_file_format_t;

typedef struct rt_buffer_s {
    size_t      used;
    size_t      size;
    char       *bytes;
} rt_buffer_t;

typedef struct rt_buffer_ctx_s {
    rt_buffer_t     *buffer;
    readstat_off_t   pos;
} rt_buffer_ctx_t;

typedef struct rt_error_s {
    readstat_value_t    received;
    readstat_value_t    expected;
    readstat_off_t      pos;
    long                var_index;
    long                obs_index;
    char                msg[256];
} rt_error_t;

typedef struct rt_parse_ctx_s {
    rt_error_t      *errors;
    long             errors_count;

    long             var_index;
    long             obs_index;

    rt_buffer_ctx_t *buffer_ctx;
} rt_parse_ctx_t;

typedef struct rt_column_s {
    char                    name[MAX_STRING];
    char                    label[MAX_STRING];
    readstat_alignment_t    alignment;
    readstat_measure_t      measure;
    readstat_types_t        type;
    double                  double_values[MAX_ROWS];
    float                   float_values[MAX_ROWS];
    char                    string_values[MAX_ROWS][MAX_STRING];
} rt_column_t;

rt_column_t _data[] = {
    { 
        .name = "var1",
        .label = "Double-precision variable",
        .type = READSTAT_TYPE_DOUBLE,
        .alignment = READSTAT_ALIGNMENT_CENTER,
        .measure = READSTAT_MEASURE_SCALE,
        .double_values = { 100.0, 10.0, -3.14159, NAN }
    },
    { 
        .name = "var2",
        .label = "Single-precision variable",
        .type = READSTAT_TYPE_FLOAT,
        .alignment = READSTAT_ALIGNMENT_CENTER,
        .measure = READSTAT_MEASURE_SCALE,
        .float_values = { 20.0, 15.0, 3.14159, NAN }
    },
    { 
        .name = "var3",
        .label = "String variable",
        .type = READSTAT_TYPE_STRING,
        .alignment = READSTAT_ALIGNMENT_LEFT,
        .measure = READSTAT_MEASURE_ORDINAL,
        .string_values = { "Hello", "Goodbye" }
    }
};

void push_error_if_doubles_differ(rt_parse_ctx_t *ctx,
        double expected,
        double received,
        const char *msg);

void push_error_if_strings_differ(rt_parse_ctx_t *ctx, 
        const char *expected,
        const char *received,
        const char *msg);

static rt_buffer_t *buffer_init() {
    rt_buffer_t *buffer = calloc(1, sizeof(rt_buffer_t));
    buffer->size = 1024;
    buffer->bytes = malloc(buffer->size);

    return buffer;
}

static void buffer_reset(rt_buffer_t *buffer) {
    buffer->used = 0;
}

static void buffer_free(rt_buffer_t *buffer) {
    free(buffer->bytes);
    free(buffer);
}

static rt_buffer_ctx_t *buffer_ctx_init(rt_buffer_t *buffer) {
    rt_buffer_ctx_t *buffer_ctx = calloc(1, sizeof(rt_buffer_ctx_t));
    buffer_ctx->buffer = buffer;
    return buffer_ctx;
}

static ssize_t write_data(const void *bytes, size_t len, void *ctx) {
    rt_buffer_t *buffer = (rt_buffer_t *)ctx;
    while (len > buffer->size - buffer->used) {
        buffer->size *= 2;
    }
    buffer->bytes = realloc(buffer->bytes, buffer->size);
    if (buffer->bytes == NULL) {
        return -1;
    }
    memcpy(buffer->bytes + buffer->used, bytes, len);
    buffer->used += len;
    return len;
}

int rt_open_handler(const char *path, void *io_ctx) {
    return 0;
}

int rt_close_handler(void *io_ctx) {
    return 0;
}

readstat_off_t rt_seek_handler(readstat_off_t offset,
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

ssize_t rt_read_handler(void *buf, size_t nbytes, void *io_ctx) {
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

readstat_error_t rt_update_handler(long file_size,
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
            sizeof(_data)/sizeof(_data[0]), var_count, 
            "Number of variables");

    push_error_if_doubles_differ(rt_ctx, 
            MAX_ROWS, obs_count, 
            "Number of observations");

    return 0;
}

static int handle_variable(int index, readstat_variable_t *variable,
                           const char *val_labels, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;
    rt_column_t *column = &_data[index];

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
    rt_column_t *column = &_data[var_index];

    rt_ctx->obs_index = obs_index;
    rt_ctx->var_index = var_index;

    if (column->type == READSTAT_TYPE_STRING) {
        push_error_if_strings_differ(rt_ctx, 
                column->string_values[obs_index],
                readstat_string_value(value),
                "String values");
    } else if (column->type == READSTAT_TYPE_DOUBLE) {
        push_error_if_doubles_differ(rt_ctx, 
                column->double_values[obs_index],
                readstat_double_value(value),
                "Double values");
    } else if (column->type == READSTAT_TYPE_FLOAT) {
        push_error_if_doubles_differ(rt_ctx, 
                column->float_values[obs_index],
                readstat_float_value(value),
                "Float values");
    }

    return 0;
}

void push_error(rt_parse_ctx_t *ctx, 
        readstat_value_t expected,
        readstat_value_t received,
        const char *msg) {
    ctx->errors = realloc(ctx->errors, (ctx->errors_count+1) * sizeof(rt_error_t));

    rt_error_t *error = &ctx->errors[ctx->errors_count];
    error->expected = expected;
    error->received = received;
    error->pos = ctx->buffer_ctx->pos;
    error->var_index = ctx->var_index;
    error->obs_index = ctx->obs_index;
    snprintf(error->msg, sizeof(error->msg), "%s", msg);

    ctx->errors_count++;
}

void push_error_if_strings_differ(rt_parse_ctx_t *ctx, 
        const char *expected,
        const char *received,
        const char *msg) {
    if ((expected == NULL || expected[0] == '\0') && 
            (received == NULL || received[0] == '\0'))
        return;

    if (expected && received && strcmp(expected, received) == 0)
        return;

    readstat_value_t expected_value = { .type = READSTAT_TYPE_STRING,
        .v = { .string_value = expected } };
    readstat_value_t received_value = { .type = READSTAT_TYPE_STRING,
        .v = { .string_value = received } };

    push_error(ctx, expected_value, received_value, msg);
}

void push_error_if_doubles_differ(rt_parse_ctx_t *ctx,
        double expected,
        double received,
        const char *msg) {
    if (isnan(expected) && isnan(received))
        return;

    if (expected == received)
        return;

    readstat_value_t expected_value = { .type = READSTAT_TYPE_DOUBLE,
        .v = { .double_value = expected } };
    readstat_value_t received_value = { .type = READSTAT_TYPE_DOUBLE,
        .v = { .double_value = received } };

    push_error(ctx, expected_value, received_value, msg);
}

readstat_error_t write_file_to_buffer(rt_buffer_t *buffer, rt_file_format_t format) {
    readstat_error_t error = READSTAT_OK;

    readstat_writer_t *writer = readstat_writer_init();
    readstat_set_data_writer(writer, &write_data);
    readstat_writer_set_file_label(writer, "ReadStat Test File");

    if (format == RT_FORMAT_DTA) {
        error = readstat_begin_writing_dta(writer, buffer, MAX_ROWS);
    } else if (format == RT_FORMAT_SAV) {
        error = readstat_begin_writing_sav(writer, buffer, MAX_ROWS);
    }

    if (error != READSTAT_OK)
        goto cleanup;

    int i, j;
    for (j=0; j<sizeof(_data)/sizeof(_data[0]); j++) {
        rt_column_t *column = &_data[j];

        readstat_variable_t *variable = readstat_add_variable(writer, 
                column->name, column->type, MAX_STRING);

        readstat_variable_set_alignment(variable, column->alignment);
        readstat_variable_set_measure(variable, column->measure);
        readstat_variable_set_label(variable, column->label);
    }

    for (i=0; i<MAX_ROWS; i++) {
        error = readstat_begin_row(writer);
        if (error != READSTAT_OK)
            goto cleanup;

        for (j=0; j<sizeof(_data)/sizeof(_data[0]); j++) {
            rt_column_t *column = &_data[j];
            readstat_variable_t *variable = readstat_get_variable(writer, j);

            if (column->type == READSTAT_TYPE_STRING ||
                    column->type == READSTAT_TYPE_LONG_STRING) {
                error = readstat_insert_string_value(writer, variable, column->string_values[i]);
            } else if (column->type == READSTAT_TYPE_DOUBLE) {
                error = readstat_insert_double_value(writer, variable, column->double_values[i]);
            } else if (column->type == READSTAT_TYPE_FLOAT) {
                error = readstat_insert_float_value(writer, variable, column->float_values[i]);
            }
            if (error != READSTAT_OK)
                goto cleanup;
        }

        error = readstat_end_row(writer);
        if (error != READSTAT_OK)
            goto cleanup;
    }
    error = readstat_end_writing(writer);
    if (error != READSTAT_OK)
        goto cleanup;

cleanup:
    readstat_writer_free(writer);

    return error;
}

void print_value(readstat_value_t value) {
    if (value.type == READSTAT_TYPE_STRING) {
        printf("%s", readstat_string_value(value));
    } else if (value.type == READSTAT_TYPE_DOUBLE) {
        printf("%lf", readstat_double_value(value));
    }
}

void print_error(rt_error_t *error) {
    printf("Test failed: %s\n", error->msg);

    printf(" * Expected: ");
    print_value(error->expected);
    printf("\n");

    printf(" * Received: ");
    print_value(error->received);
    printf("\n");

    if (error->obs_index != -1) {
        printf(" * Row: %ld\n", error->obs_index + 1);
    }

    if (error->var_index != -1) {
        printf(" * Column: %ld\n", error->var_index + 1);
    }
}

readstat_error_t read_file(rt_parse_ctx_t *parse_ctx, rt_file_format_t format) {
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

    if (format == RT_FORMAT_DTA) {
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

int main(int argc, char *argv[]) {
    rt_buffer_t *buffer = buffer_init();
    readstat_error_t error = READSTAT_OK;

    rt_file_format_t f;
    for (f=RT_FORMAT_DTA; f<=RT_FORMAT_SAV; f++) {
        error = write_file_to_buffer(buffer, f);
        if (error != READSTAT_OK)
            goto cleanup;

        rt_parse_ctx_t *parse_ctx = calloc(1, sizeof(rt_parse_ctx_t));
        parse_ctx->buffer_ctx = buffer_ctx_init(buffer);

        error = read_file(parse_ctx, f);
        if (error != READSTAT_OK)
            goto cleanup;

        if (parse_ctx->errors_count) {
            int i;
            for (i=0; i<parse_ctx->errors_count; i++) {
                print_error(&parse_ctx->errors[i]);
            }
            return 1;
        }

        buffer_reset(buffer);
    }

cleanup:
    if (error != READSTAT_OK) {
        printf("Error running test: %s\n", readstat_error_message(error));
        return 1;
    }

    return 0;
}
