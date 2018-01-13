#include <stdlib.h>

#include "../readstat.h"

#include "../test/test_types.h"
#include "../test/test_buffer.h"
#include "../test/test_buffer_io.h"

static int handle_info(int obs_count, int var_count, void *ctx) {
    return READSTAT_HANDLER_OK;
}

static int handle_metadata(const char *file_label, const char *encoding, time_t timestamp, long format_version, void *ctx) {
    return READSTAT_HANDLER_OK;
}

static int handle_note(int index, const char *note, void *ctx) {
    return READSTAT_HANDLER_OK;
}

static int handle_fweight(readstat_variable_t *variable, void *ctx) {
    return READSTAT_HANDLER_OK;
}

static int handle_variable(int index, readstat_variable_t *variable,
                           const char *val_labels, void *ctx) {
    return READSTAT_HANDLER_OK;
}

static int handle_value(int obs_index, readstat_variable_t *variable, readstat_value_t value, void *ctx) {
    return READSTAT_HANDLER_OK;
}

static int handle_value_label(const char *val_labels, readstat_value_t value, const char *label, void *ctx) {
    return READSTAT_HANDLER_OK;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    readstat_parser_t *parser = readstat_parser_init();
    rt_buffer_ctx_t *buffer_ctx = calloc(1, sizeof(rt_buffer_ctx_t));
    buffer_ctx->buffer = calloc(1, sizeof(rt_buffer_t));
    buffer_ctx->buffer->size = Size;
    buffer_ctx->buffer->used = Size;
    buffer_ctx->buffer->bytes = Data;

    readstat_set_open_handler(parser, rt_open_handler);
    readstat_set_close_handler(parser, rt_close_handler);
    readstat_set_seek_handler(parser, rt_seek_handler);
    readstat_set_read_handler(parser, rt_read_handler);
    readstat_set_update_handler(parser, rt_update_handler);

    readstat_set_io_ctx(parser, buffer_ctx);

    readstat_set_info_handler(parser, &handle_info);
    readstat_set_metadata_handler(parser, &handle_metadata);
    readstat_set_note_handler(parser, &handle_note);
    readstat_set_variable_handler(parser, &handle_variable);
    readstat_set_fweight_handler(parser, &handle_fweight);
    readstat_set_value_handler(parser, &handle_value);
    readstat_set_value_label_handler(parser, &handle_value_label);

    readstat_parse_sav(parser, NULL, NULL);

    readstat_parser_free(parser);
    free(buffer_ctx->buffer);
    free(buffer_ctx);

    return 0;  // Non-zero return values are reserved for future use.
}
