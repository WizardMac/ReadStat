#ifndef __PRODUCE_CSV_COLUMN_HEADER_H
#define __PRODUCE_CSV_COLUMN_HEADER_H

#include "../../readstat.h"

void produce_column_header(void *s, size_t len, void *data);

typedef struct csv_metadata {
    int pass;
    long rows;
    long columns;
    long _columns;
    long _rows;
    int output_format;
    size_t* column_width;
    int open_row;
    readstat_parser_t *parser;
    void *user_ctx;
    readstat_variable_t* variables;
    int* is_date;
    struct json_metadata* json_md;
} csv_metadata;

#endif
