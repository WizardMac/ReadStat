#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../readstat.h"

#include "test_buffer.h"
#include "test_buffer_io.h"
#include "test_readstat.h"
#include "test_sas.h"

/* Needs more than 15 bits as a length */
#define RT_SAS7BCAT_LONG_STRING_LEN  33000
/* Three such label sets fill most of a 64 KiB catalog page */
#define RT_SAS7BCAT_SET_LABELS_COUNT   340

typedef enum rt_sas7bcat_case_e {
    RT_SAS7BCAT_LABEL_SETS_PAST_32K,
    RT_SAS7BCAT_LONG_LABEL,
    RT_SAS7BCAT_LONG_STRING_VALUE,
    RT_SAS7BCAT_CASES_COUNT
} rt_sas7bcat_case_t;

typedef struct rt_sas7bcat_ctx_s {
    rt_sas7bcat_case_t  test_case;
    const char         *long_string;
    long                labels_count;
    long                errors_count;
} rt_sas7bcat_ctx_t;

long sas_file_format_version(long format_code) {
    if ((format_code & RT_FORMAT_XPORT_5))
        return 5;

    if ((format_code & RT_FORMAT_XPORT_8))
        return 8;

    if ((format_code & RT_FORMAT_SAS7BDAT_32BIT))
        return 8;

    return 9;
}

static ssize_t sas7bcat_write_data(const void *bytes, size_t len, void *ctx) {
    rt_buffer_t *buffer = (rt_buffer_t *)ctx;
    buffer_grow(buffer, len);
    if (buffer->bytes == NULL) {
        return -1;
    }
    memcpy(buffer->bytes + buffer->used, bytes, len);
    buffer->used += len;
    return len;
}

static void sas7bcat_add_label_sets(readstat_writer_t *writer, rt_sas7bcat_ctx_t *rt_ctx) {
    readstat_label_set_t *label_set = NULL;
    char name[16];
    char label[32];
    int i, j;

    if (rt_ctx->test_case == RT_SAS7BCAT_LABEL_SETS_PAST_32K) {
        for (i=0; i<3; i++) {
            snprintf(name, sizeof(name), "SET%c", 'A' + i);
            label_set = readstat_add_label_set(writer, READSTAT_TYPE_DOUBLE, name);
            for (j=0; j<RT_SAS7BCAT_SET_LABELS_COUNT; j++) {
                snprintf(label, sizeof(label), "Label %d of set %c", j, 'A' + i);
                readstat_label_double_value(label_set, j, label);
            }
        }
    } else if (rt_ctx->test_case == RT_SAS7BCAT_LONG_LABEL) {
        label_set = readstat_add_label_set(writer, READSTAT_TYPE_DOUBLE, "LONGLBL");
        readstat_label_double_value(label_set, 1, rt_ctx->long_string);
        readstat_label_double_value(label_set, 2, "Two");
    } else if (rt_ctx->test_case == RT_SAS7BCAT_LONG_STRING_VALUE) {
        label_set = readstat_add_label_set(writer, READSTAT_TYPE_STRING, "$LONGVAL");
        readstat_label_string_value(label_set, rt_ctx->long_string, "Long");
        readstat_label_string_value(label_set, "b", "Short");
    }
}

static long sas7bcat_expected_labels_count(rt_sas7bcat_case_t test_case) {
    if (test_case == RT_SAS7BCAT_LABEL_SETS_PAST_32K)
        return 3 * RT_SAS7BCAT_SET_LABELS_COUNT;

    return 2;
}

static int sas7bcat_handle_value_label(const char *val_labels, readstat_value_t value,
        const char *label, void *ctx) {
    rt_sas7bcat_ctx_t *rt_ctx = (rt_sas7bcat_ctx_t *)ctx;
    const char *expected_label = NULL;
    char expected[32];

    if (rt_ctx->test_case == RT_SAS7BCAT_LABEL_SETS_PAST_32K) {
        if (strlen(val_labels) == 4 && strncmp(val_labels, "SET", 3) == 0) {
            snprintf(expected, sizeof(expected), "Label %d of set %c",
                    (int)readstat_double_value(value), val_labels[3]);
            expected_label = expected;
        }
    } else if (rt_ctx->test_case == RT_SAS7BCAT_LONG_LABEL) {
        if (readstat_double_value(value) == 1) {
            expected_label = rt_ctx->long_string;
        } else if (readstat_double_value(value) == 2) {
            expected_label = "Two";
        }
    } else if (rt_ctx->test_case == RT_SAS7BCAT_LONG_STRING_VALUE) {
        const char *string_value = readstat_string_value(value);
        if (string_value && strcmp(string_value, rt_ctx->long_string) == 0) {
            expected_label = "Long";
        } else if (string_value && strcmp(string_value, "b") == 0) {
            expected_label = "Short";
        }
    }

    if (expected_label == NULL || strcmp(label, expected_label) != 0)
        rt_ctx->errors_count++;

    rt_ctx->labels_count++;

    return READSTAT_HANDLER_OK;
}

static int test_sas7bcat_round_trip(rt_buffer_ctx_t *buffer_ctx, rt_sas7bcat_ctx_t *rt_ctx,
        int is_64bit) {
    readstat_error_t error = READSTAT_OK;
    readstat_writer_t *writer = readstat_writer_init();
    readstat_parser_t *parser = NULL;

    buffer_ctx_reset(buffer_ctx);
    readstat_set_data_writer(writer, &sas7bcat_write_data);
    readstat_writer_set_file_format_is_64bit(writer, is_64bit);
    sas7bcat_add_label_sets(writer, rt_ctx);

    error = readstat_begin_writing_sas7bcat(writer, buffer_ctx->buffer);
    if (error == READSTAT_OK)
        error = readstat_end_writing(writer);
    readstat_writer_free(writer);
    if (error != READSTAT_OK)
        goto cleanup;

    parser = readstat_parser_init();
    readstat_set_open_handler(parser, rt_open_handler);
    readstat_set_close_handler(parser, rt_close_handler);
    readstat_set_seek_handler(parser, rt_seek_handler);
    readstat_set_read_handler(parser, rt_read_handler);
    readstat_set_update_handler(parser, rt_update_handler);
    readstat_set_io_ctx(parser, buffer_ctx);
    readstat_set_value_label_handler(parser, &sas7bcat_handle_value_label);

    error = readstat_parse_sas7bcat(parser, NULL, rt_ctx);
    readstat_parser_free(parser);

cleanup:
    if (error != READSTAT_OK) {
        printf("Error running SAS7BCAT test %d (64-bit=%d): %s\n",
                rt_ctx->test_case, is_64bit, readstat_error_message(error));
        return 1;
    }
    if (rt_ctx->errors_count ||
            rt_ctx->labels_count != sas7bcat_expected_labels_count(rt_ctx->test_case)) {
        printf("Error running SAS7BCAT test %d (64-bit=%d): read %ld of %ld labels, %ld wrong\n",
                rt_ctx->test_case, is_64bit, rt_ctx->labels_count,
                sas7bcat_expected_labels_count(rt_ctx->test_case), rt_ctx->errors_count);
        return 1;
    }
    return 0;
}

/* Catalogs whose blocks, labels or string values need more than 15 bits as
 * an offset or length */
int test_sas7bcat_large_label_sets(void) {
    rt_buffer_t *buffer = buffer_init();
    rt_buffer_ctx_t *buffer_ctx = buffer_ctx_init(buffer);
    char *long_string = malloc(RT_SAS7BCAT_LONG_STRING_LEN + 1);
    int failed = 0;
    int test_case, is_64bit;

    memset(long_string, 'x', RT_SAS7BCAT_LONG_STRING_LEN);
    long_string[RT_SAS7BCAT_LONG_STRING_LEN] = '\0';

    for (test_case=0; test_case<RT_SAS7BCAT_CASES_COUNT; test_case++) {
        for (is_64bit=0; is_64bit<=1; is_64bit++) {
            rt_sas7bcat_ctx_t rt_ctx = {
                .test_case = test_case,
                .long_string = long_string
            };
            failed |= test_sas7bcat_round_trip(buffer_ctx, &rt_ctx, is_64bit);
        }
    }

    free(long_string);
    free(buffer_ctx);
    buffer_free(buffer);
    return failed;
}
