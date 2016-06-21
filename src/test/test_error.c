#include <stdlib.h>

#include "../readstat.h"

#include "test_types.h"
#include "test_error.h"
#include "test_readstat.h"

void push_error(rt_parse_ctx_t *ctx, 
        readstat_value_t expected,
        readstat_value_t received,
        const char *msg) {
    ctx->errors = realloc(ctx->errors, (ctx->errors_count+1) * sizeof(rt_error_t));

    rt_error_t *error = &ctx->errors[ctx->errors_count];
    error->expected = expected;
    error->received = received;
    error->file = ctx->file;
    error->file_format = ctx->file_format;
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

void push_error_if_values_differ(rt_parse_ctx_t *ctx, 
        readstat_value_t expected,
        readstat_value_t received,
        const char *msg) {
    readstat_types_t expected_type = readstat_value_type(expected);
    readstat_types_t received_type = readstat_value_type(received);
    if (expected_type == READSTAT_TYPE_STRING || expected_type == READSTAT_TYPE_LONG_STRING) {
        if (received_type == READSTAT_TYPE_STRING || received_type == READSTAT_TYPE_LONG_STRING) {
            push_error_if_strings_differ(ctx,
                    readstat_string_value(expected),
                    readstat_string_value(received),
                    msg);
        } else {
            push_error(ctx, expected, received, msg);
        }
    } else if (received_type != READSTAT_TYPE_STRING && received_type != READSTAT_TYPE_LONG_STRING) {
        if (readstat_value_tag(expected) || readstat_value_tag(received)) {
            if (readstat_value_tag(expected) != readstat_value_tag(received)) {
                push_error(ctx, expected, received, msg);
            }
        } else if (received_type == READSTAT_TYPE_DOUBLE || received_type == READSTAT_TYPE_FLOAT) {
            push_error_if_doubles_differ(ctx,
                    readstat_double_value(expected),
                    readstat_double_value(received),
                    msg);
        } else {
            if (readstat_int32_value(expected) != readstat_int32_value(received)) {
                push_error(ctx, expected, received, msg);
            }
        }
    } else {
        push_error(ctx, expected, received, msg);
    }
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

void push_error_if_codes_differ(rt_parse_ctx_t *ctx,
        readstat_error_t expected,
        readstat_error_t received) {
    if (expected == received)
        return;

    readstat_value_t expected_value = { .type = READSTAT_TYPE_STRING,
        .v = { .string_value = readstat_error_message(expected) } };
    readstat_value_t received_value = { .type = READSTAT_TYPE_STRING,
        .v = { .string_value = readstat_error_message(received) } };

    push_error(ctx, expected_value, received_value, "Error codes");
}

static void print_value(readstat_value_t value) {
    if (value.type == READSTAT_TYPE_STRING) {
        printf("%s", readstat_string_value(value));
    } else if (value.tag) {
        printf(".%c (tagged)", readstat_value_tag(value));
    } else if (value.type == READSTAT_TYPE_DOUBLE) {
        printf("%lf (double)", readstat_double_value(value));
    } else if (value.type == READSTAT_TYPE_FLOAT) {
        printf("%f (float)", readstat_float_value(value));
    } else if (value.type == READSTAT_TYPE_CHAR) {
        printf("%hhd (int8)", readstat_char_value(value));
    } else if (value.type == READSTAT_TYPE_INT16) {
        printf("%hd (int16)", readstat_int16_value(value));
    } else if (value.type == READSTAT_TYPE_INT32) {
        printf("%d (int32)", readstat_int32_value(value));
    }
}

void print_error(rt_error_t *error) {
    if (error->file) {
        printf("Test \"%s\" failed: %s\n", error->file->label, error->msg);
    } else {
        printf("Test failed: %s\n", error->msg);
    }

    printf(" * Format: 0x%04lx\n", error->file_format);

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
