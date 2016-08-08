#include <stdlib.h>

#include "../readstat.h"

#include "test_types.h"
#include "test_error.h"
#include "test_readstat.h"

int strings_equal(const char *expected, const char *received) {
    if ((expected == NULL || expected[0] == '\0') && 
            (received == NULL || received[0] == '\0'))
        return 1;

    return (expected && received && strcmp(expected, received) == 0);
}

int doubles_equal(double expected, double received) {
    if (isnan(expected) && isnan(received))
        return 1;

    return (expected == received);
}

static readstat_value_t copy_value(rt_parse_ctx_t *ctx, readstat_value_t value) {
    if (value.type == READSTAT_TYPE_STRING) {
        if (value.v.string_value) {
            size_t len = strlen(value.v.string_value);
            ctx->strings = realloc(ctx->strings, ctx->strings_len + len + 1);
            memcpy(&ctx->strings[ctx->strings_len], value.v.string_value, len + 1);
            value.v.string_value = &ctx->strings[ctx->strings_len];
            ctx->strings_len += len + 1;
        }
    }
    return value;
}

void push_error(rt_parse_ctx_t *ctx, 
        readstat_value_t expected,
        readstat_value_t received,
        const char *msg) {
    ctx->errors = realloc(ctx->errors, (ctx->errors_count+1) * sizeof(rt_error_t));

    rt_error_t *error = &ctx->errors[ctx->errors_count];
    error->expected = copy_value(ctx, expected);
    error->received = copy_value(ctx, received);
    error->file = ctx->file;
    error->file_format = ctx->file_format;
    error->file_extension = ctx->file_extension;
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
    if (strings_equal(expected, received))
        return;

    readstat_value_t expected_value = { .type = READSTAT_TYPE_STRING,
        .v = { .string_value = expected } };
    readstat_value_t received_value = { .type = READSTAT_TYPE_STRING,
        .v = { .string_value = received } };

    push_error(ctx, expected_value, received_value, msg);
}

void push_error_if_strings_differ_n(rt_parse_ctx_t *ctx, 
        const char *expected,
        const char *received,
        size_t len,
        const char *msg) {
    if ((expected == NULL || expected[0] == '\0') && 
            (received == NULL || received[0] == '\0'))
        return;

    if (expected && received && strncmp(expected, received, len) == 0)
        return;

    readstat_value_t expected_value = { .type = READSTAT_TYPE_STRING,
        .v = { .string_value = expected } };
    readstat_value_t received_value = { .type = READSTAT_TYPE_STRING,
        .v = { .string_value = received } };

    push_error(ctx, expected_value, received_value, msg);
}

int values_equal(readstat_value_t expected, readstat_value_t received) {
    readstat_type_t expected_type = readstat_value_type(expected);
    readstat_type_t received_type = readstat_value_type(received);
    if (expected_type == READSTAT_TYPE_STRING) {
        if (received_type == READSTAT_TYPE_STRING) {
            return strings_equal(readstat_string_value(expected), readstat_string_value(received));
        } else {
            return 0;
        }
    } else if (received_type != READSTAT_TYPE_STRING) {
        if (readstat_value_is_tagged_missing(expected) || readstat_value_is_tagged_missing(received)) {
            return readstat_value_tag(expected) == readstat_value_tag(received);
        } else if (readstat_value_is_system_missing(expected) || readstat_value_is_system_missing(received)) {
            return readstat_value_is_system_missing(expected) == readstat_value_is_system_missing(received);
        } else if (received_type == READSTAT_TYPE_DOUBLE || received_type == READSTAT_TYPE_FLOAT) {
            return doubles_equal(readstat_double_value(expected), readstat_double_value(received));
        } else {
            return readstat_int32_value(expected) == readstat_int32_value(received);
        }
    } else {
        return 0;
    }
    return 1;
}


void push_error_if_values_differ(rt_parse_ctx_t *ctx, 
        readstat_value_t expected,
        readstat_value_t received,
        const char *msg) {
    if (values_equal(expected, received))
        return;

    push_error(ctx, expected, received, msg);
}

void push_error_if_doubles_differ(rt_parse_ctx_t *ctx,
        double expected,
        double received,
        const char *msg) {
    if (doubles_equal(expected, received))
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
        const char *string = readstat_string_value(value);
        if (string) {
            printf("\"%s\" (length=%ld)", string, strlen(string));
        } else {
            printf("(null)");
        }
    } else if (value.tag) {
        printf(".%c (tagged)", readstat_value_tag(value));
    } else if (value.type == READSTAT_TYPE_DOUBLE) {
        printf("%lf (double)", readstat_double_value(value));
    } else if (value.type == READSTAT_TYPE_FLOAT) {
        printf("%f (float)", readstat_float_value(value));
    } else if (value.type == READSTAT_TYPE_INT8) {
        printf("%hhd (int8)", readstat_int8_value(value));
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

    printf(" * Format: %s (0x%04lx)\n", error->file_extension, error->file_format);

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
