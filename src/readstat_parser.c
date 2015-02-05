
#include <stdlib.h>
#include "readstat.h"

readstat_parser_t *readstat_parser_init() {
    readstat_parser_t *parser = calloc(1, sizeof(readstat_parser_t));
    return parser;
}

void readstat_parser_free(readstat_parser_t *parser) {
    if (parser)
        free(parser);
}

readstat_error_t readstat_set_info_handler(readstat_parser_t *parser, readstat_handle_info_callback info_cb) {
    parser->info_cb = info_cb;
    return READSTAT_OK;
}

readstat_error_t readstat_set_variable_handler(readstat_parser_t *parser, readstat_handle_variable_callback variable_cb) {
    parser->variable_cb = variable_cb;
    return READSTAT_OK;
}

readstat_error_t readstat_set_value_handler(readstat_parser_t *parser, readstat_handle_value_callback value_cb) {
    parser->value_cb = value_cb;
    return READSTAT_OK;
}

readstat_error_t readstat_set_value_label_handler(readstat_parser_t *parser, readstat_handle_value_label_callback label_cb) {
    parser->value_label_cb = label_cb;
    return READSTAT_OK;
}

readstat_error_t readstat_set_error_handler(readstat_parser_t *parser, readstat_handle_error_callback error_cb) {
    parser->error_cb = error_cb;
    return READSTAT_OK;
}

rdata_parser_t *rdata_parser_init() {
    rdata_parser_t *parser = calloc(1, sizeof(rdata_parser_t));
    return parser;
}

void rdata_parser_free(rdata_parser_t *parser) {
    if (parser)
        free(parser);
}

readstat_error_t rdata_set_table_handler(rdata_parser_t *parser, rdata_handle_table_callback table_cb) {
    parser->table_cb = table_cb;
    return READSTAT_OK;
}

readstat_error_t rdata_set_column_handler(rdata_parser_t *parser, rdata_handle_column_callback column_cb) {
    parser->column_cb = column_cb;
    return READSTAT_OK;
}

readstat_error_t rdata_set_column_name_handler(rdata_parser_t *parser, rdata_handle_column_name_callback column_name_cb) {
    parser->column_name_cb = column_name_cb;
    return READSTAT_OK;
}

readstat_error_t rdata_set_text_value_handler(rdata_parser_t *parser, rdata_handle_text_value_callback text_value_cb) {
    parser->text_value_cb = text_value_cb;
    return READSTAT_OK;
}

readstat_error_t rdata_set_value_label_handler(rdata_parser_t *parser, rdata_handle_text_value_callback value_label_cb) {
    parser->value_label_cb = value_label_cb;
    return READSTAT_OK;
}

readstat_error_t rdata_set_error_handler(rdata_parser_t *parser, readstat_handle_error_callback error_cb) {
    parser->error_cb = error_cb;
    return READSTAT_OK;
}
