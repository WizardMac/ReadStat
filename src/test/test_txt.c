#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../readstat.h"

#include "test_buffer.h"
#include "test_buffer_io.h"
#include "test_txt.h"

typedef readstat_schema_t *(*rt_schema_parser_t)(readstat_parser_t *parser,
        const char *filepath, void *user_ctx, readstat_error_t *outError);

typedef struct rt_txt_test_s {
    const char         *label;
    rt_schema_parser_t  schema_parser;
    const char         *schema;
    const char         *data;
    readstat_error_t    parse_error;
} rt_txt_test_t;

typedef struct rt_txt_ctx_s {
    int     values[2];
    int     value_count;
    int     error_count;
} rt_txt_ctx_t;

/* Every schema reads x = 1 and y = 2 from its data, or must be rejected by
 * readstat_parse_txt because an entry lies outside the observation */
static rt_txt_test_t _txt_tests[] = {
    {
        .label = "_line() within _lines()",
        .schema_parser = &readstat_parse_stata_dictionary,
        .schema = "dictionary {\n_lines(2)\nbyte x %1f\n_line(2)\n_column(1)\nbyte y %1f\n}\n",
        .data = "1\n2\n",
        .parse_error = READSTAT_OK
    },
    {
        .label = "_line() past the default single line",
        .schema_parser = &readstat_parse_stata_dictionary,
        .schema = "dictionary {\nbyte x %1f\n_line(2)\n_column(1)\nbyte y %1f\n}\n",
        .data = "1\n2\n",
        .parse_error = READSTAT_ERROR_PARSE
    },
    {
        .label = "_line() past _lines()",
        .schema_parser = &readstat_parse_stata_dictionary,
        .schema = "dictionary {\n_lines(2)\nbyte x %1f\n_line(3)\n_column(1)\nbyte y %1f\n}\n",
        .data = "1\n2\n",
        .parse_error = READSTAT_ERROR_PARSE
    },
    {
        .label = "_newline past the default single line",
        .schema_parser = &readstat_parse_stata_dictionary,
        .schema = "dictionary {\nbyte x %1f\n_newline\n_column(1)\nbyte y %1f\n}\n",
        .data = "1\n2\n",
        .parse_error = READSTAT_ERROR_PARSE
    },
    {
        .label = "_line(0)",
        .schema_parser = &readstat_parse_stata_dictionary,
        .schema = "dictionary {\n_line(0)\nbyte x %1f\n}\n",
        .data = "1\n2\n",
        .parse_error = READSTAT_ERROR_PARSE
    },
    {
        .label = "_lines(0)",
        .schema_parser = &readstat_parse_stata_dictionary,
        .schema = "dictionary {\n_lines(0)\nbyte x %1f\n}\n",
        .data = "1\n2\n",
        .parse_error = READSTAT_ERROR_PARSE
    },
    {
        .label = "_column(0)",
        .schema_parser = &readstat_parse_stata_dictionary,
        .schema = "dictionary {\nbyte x %1f\n_column(0)\nbyte y %1f\n}\n",
        .data = "12\n",
        .parse_error = READSTAT_ERROR_PARSE
    },
    {
        .label = "SAS INPUT #2",
        .schema_parser = &readstat_parse_sas_commands,
        .schema = "INPUT x 1-1 #2 y 1-1;\n",
        .data = "1\n2\n",
        .parse_error = READSTAT_OK
    },
    {
        .label = "SAS INPUT #0",
        .schema_parser = &readstat_parse_sas_commands,
        .schema = "INPUT #0 x 1-1;\n",
        .data = "1\n2\n",
        .parse_error = READSTAT_ERROR_PARSE
    },
    {
        .label = "SAS INPUT column 0",
        .schema_parser = &readstat_parse_sas_commands,
        .schema = "INPUT x 1-1 y 0-1;\n",
        .data = "12\n",
        .parse_error = READSTAT_ERROR_PARSE
    },
    {
        .label = "SAS INPUT reversed column range",
        .schema_parser = &readstat_parse_sas_commands,
        .schema = "INPUT x 1-1 y 3-1;\n",
        .data = "12\n",
        .parse_error = READSTAT_ERROR_PARSE
    },
    {
        .label = "SAS INFILE DLM",
        .schema_parser = &readstat_parse_sas_commands,
        .schema = "INFILE \"test.txt\" DLM=\",\";\nINPUT x 1-1 y 3-3;\n",
        .data = "1,2\n",
        .parse_error = READSTAT_OK
    },
    {
        .label = "SPSS DATA LIST /1",
        .schema_parser = &readstat_parse_spss_commands,
        .schema = "DATA LIST FIXED /1 x 1 y 2.\n",
        .data = "12\n",
        .parse_error = READSTAT_OK
    },
    {
        .label = "SPSS DATA LIST /0",
        .schema_parser = &readstat_parse_spss_commands,
        .schema = "DATA LIST FIXED /0 x 1 y 2.\n",
        .data = "12\n",
        .parse_error = READSTAT_ERROR_PARSE
    }
};

static int handle_value(int obs_index, readstat_variable_t *variable,
        readstat_value_t value, void *ctx) {
    rt_txt_ctx_t *txt_ctx = (rt_txt_ctx_t *)ctx;
    if (txt_ctx->value_count < sizeof(txt_ctx->values)/sizeof(txt_ctx->values[0]))
        txt_ctx->values[txt_ctx->value_count] = readstat_int32_value(value);
    txt_ctx->value_count++;
    return READSTAT_HANDLER_OK;
}

static void handle_error(const char *error_message, void *ctx) {
    rt_txt_ctx_t *txt_ctx = (rt_txt_ctx_t *)ctx;
    txt_ctx->error_count++;
}

static void buffer_set_string(rt_buffer_ctx_t *buffer_ctx, const char *string) {
    size_t len = strlen(string);
    buffer_ctx_reset(buffer_ctx);
    buffer_grow(buffer_ctx->buffer, len);
    memcpy(buffer_ctx->buffer->bytes, string, len);
    buffer_ctx->buffer->used = len;
}

static int test_parse_txt(rt_txt_test_t *test, rt_buffer_ctx_t *buffer_ctx) {
    readstat_error_t error = READSTAT_OK;
    rt_txt_ctx_t txt_ctx = { .value_count = 0 };
    readstat_schema_t *schema = NULL;
    int failed = 0;

    readstat_parser_t *parser = readstat_parser_init();
    readstat_set_open_handler(parser, rt_open_handler);
    readstat_set_close_handler(parser, rt_close_handler);
    readstat_set_seek_handler(parser, rt_seek_handler);
    readstat_set_read_handler(parser, rt_read_handler);
    readstat_set_update_handler(parser, rt_update_handler);
    readstat_set_io_ctx(parser, buffer_ctx);
    readstat_set_value_handler(parser, &handle_value);
    readstat_set_error_handler(parser, &handle_error);

    buffer_set_string(buffer_ctx, test->schema);
    schema = test->schema_parser(parser, "test.schema", &txt_ctx, &error);
    if (schema == NULL) {
        printf("Error running test \"%s\": schema failed to parse: %s\n",
                test->label, readstat_error_message(error));
        failed = 1;
        goto cleanup;
    }

    buffer_set_string(buffer_ctx, test->data);
    error = readstat_parse_txt(parser, "test.txt", schema, &txt_ctx);
    if (error != test->parse_error) {
        printf("Error running test \"%s\": expected \"%s\", got \"%s\"\n",
                test->label, readstat_error_message(test->parse_error),
                readstat_error_message(error));
        failed = 1;
    } else if (error != READSTAT_OK) {
        if (txt_ctx.error_count == 0) {
            printf("Error running test \"%s\": no error message\n", test->label);
            failed = 1;
        }
        if (txt_ctx.value_count != 0) {
            printf("Error running test \"%s\": got %d values from a rejected schema\n",
                    test->label, txt_ctx.value_count);
            failed = 1;
        }
    } else if (txt_ctx.error_count != 0 || txt_ctx.value_count != 2 ||
            txt_ctx.values[0] != 1 || txt_ctx.values[1] != 2) {
        printf("Error running test \"%s\": expected values 1, 2\n", test->label);
        failed = 1;
    }

cleanup:
    if (schema)
        readstat_schema_free(schema);
    readstat_parser_free(parser);
    return failed;
}

int test_parse_txt_schema_bounds(void) {
    rt_buffer_t *buffer = buffer_init();
    rt_buffer_ctx_t *buffer_ctx = buffer_ctx_init(buffer);
    int failed = 0;
    int i;

    for (i=0; i<sizeof(_txt_tests)/sizeof(_txt_tests[0]); i++) {
        failed |= test_parse_txt(&_txt_tests[i], buffer_ctx);
    }

    free(buffer_ctx);
    buffer_free(buffer);
    return failed;
}
