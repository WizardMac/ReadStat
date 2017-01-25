#include <csv.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "../../readstat.h"

#include "produce_csv_value.h"
#include "produce_csv_column_header.h"
#include "../util/readstat_dta_days.h"
#include "json_metadata.h"

#define UNUSED(x) (void)(x)

void csv_metadata_cell(void *s, size_t len, void *data)
{
    struct csv_metadata *c = (struct csv_metadata *)data;
    if (c->rows == 0) {
        c->variables = realloc(c->variables, (c->columns+1) * sizeof(readstat_variable_t));
        c->is_date = realloc(c->is_date, (c->columns+1) * sizeof(int));
        produce_column_header(s, len, data);
    } else if (c->rows >= 1 && c->parser->value_handler) {
        produce_csv_column_value(s, len, data);
    }
    if (c->rows >= 1 && c->pass == 1) {
        size_t w = c->column_width[c->columns];
        c->column_width[c->columns] = (len>w) ? len : w;
    }
    c->open_row = 1;
    c->columns++;
}

void csv_metadata_row(int cc, void *data)
{
    UNUSED(cc);
    struct csv_metadata *c = (struct csv_metadata *)data;
    c->rows++;
    if (c->rows == 1 && c->pass == 1) {
        c->column_width = malloc(c->columns * sizeof(size_t));
        for (int i=0; i<c->columns; i++) {
            c->column_width[i] = 1;
        }
        c->_columns = c->columns;
    }
    c->columns = 0;
    c->open_row = 0;
}

readstat_error_t readstat_parse_csv(readstat_parser_t *parser, const char *path, const char *jsonpath, struct csv_metadata* md, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = parser->io;
    size_t file_size = 0;
    size_t bytes_read;
    struct csv_parser csvparser;
    struct csv_parser *p = &csvparser;
    char buf[BUFSIZ];
    size_t* column_width = md->column_width;
    md->pass = column_width ? 2 : 1;
    md->open_row = 0;
    md->columns = 0;
    md->_rows = md->rows;
    md->rows = 0;
    md->parser = parser;
    md->user_ctx = user_ctx;
    md->json_md = NULL;

    if ((md->json_md = get_json_metadata(jsonpath)) == NULL) {
        fprintf(stderr, "Could not get JSON metadata\n");
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if (io->open(path, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    file_size = io->seek(0, READSTAT_SEEK_END, io->io_ctx);
    if (file_size == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if (io->seek(0, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if (csv_init(p, CSV_APPEND_NULL) != 0)
    {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }
    unsigned char sep = get_separator(md->json_md);
    csv_set_delim(p, sep);
    
    while ((bytes_read = io->read(buf, sizeof(buf), io->io_ctx)) > 0)
    {
        if (csv_parse(p, buf, bytes_read, csv_metadata_cell, csv_metadata_row, md) != bytes_read)
        {
            fprintf(stderr, "Error while parsing file: %s\n", csv_strerror(csv_error(p)));
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
    }
    csv_fini(p, csv_metadata_cell, csv_metadata_row, md);
    if (!md->open_row) {
        md->rows--;
    }
    if (parser->info_handler && md->pass == 1) {
        parser->info_handler(md->rows, md->_columns, user_ctx);
    }

cleanup:
    if (md->variables) {
        free(md->variables);
        md->variables = NULL;
    }
    if (md->is_date) {
        free(md->is_date);
        md->is_date = NULL;
    }
    if (md->json_md) {
        free_json_metadata(md->json_md);
        md->json_md = NULL;
    }
    csv_free(p);
    io->close(io->io_ctx);
    return retval;
}
