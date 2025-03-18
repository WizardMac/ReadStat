#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include <csv.h>

#include "../../readstat.h"

#include "../util/readstat_dta_days.h"
#include "json_metadata.h"
#include "read_module.h"
#include "csv_metadata.h"

#include "mod_csv.h"
#include "mod_dta.h"
#include "mod_sav.h"

#define UNUSED(x) (void)(x)

rs_read_module_t *rs_read_module_for_filename(rs_read_module_t *modules, long module_count, int output_format) {
    int i;
    for (i=0; i<module_count; i++) {
        if (modules[i].format == output_format)
            return &modules[i];
    }
    return NULL;
}

static void produce_column_header(struct csv_metadata *c, void *s, size_t len) {
    char* column = (char*)s;
    readstat_variable_t* var = &c->variables[c->columns];
    memset(var, 0, sizeof(readstat_variable_t));

    extract_metadata_type_t coltype = column_type(c->json_md, column, c->output_format);
    switch (coltype) {
    case EXTRACT_METADATA_TYPE_STRING:
        var->alignment = READSTAT_ALIGNMENT_LEFT;
    break;
    case EXTRACT_METADATA_TYPE_NUMERIC:
        var->alignment = READSTAT_ALIGNMENT_RIGHT;
    break;
    default:
        var->alignment = READSTAT_ALIGNMENT_LEFT;
    }

    extract_metadata_format_t colformat = column_format(c->json_md, column);
    c->is_date[c->columns] = colformat == EXTRACT_METADATA_FORMAT_DATE;
    c->is_date_time[c->columns] = colformat == EXTRACT_METADATA_FORMAT_DATE_TIME;
    if (c->output_module->header) {
        c->output_module->header(c, column, var);
    }
    if (c->pass == 2 && coltype == EXTRACT_METADATA_TYPE_STRING) {
        var->storage_width = c->column_width[c->columns];
    }

    var->index = c->columns;
    copy_variable_property(c->json_md, column, "label", var->label, sizeof(var->label));
    snprintf(var->name, sizeof(var->name), "%.*s", (int)len, column);

    if (c->output_module->missingness) {
        c->output_module->missingness(c, column);
    }
    if (c->output_module->value_label && c->handle.value_label) {
        c->output_module->value_label(c, column);
    }

    if (c->handle.variable) {
        c->handle.variable(c->columns, var, column, c->user_ctx);
    }
}

static void csv_metadata_cell(void *s, size_t len, void *data)
{
    struct csv_metadata *c = (struct csv_metadata *)data;
    if (c->rows == 0) {
        c->variables = realloc(c->variables, (c->columns+1) * sizeof(readstat_variable_t));
        c->is_date = realloc(c->is_date, (c->columns+1) * sizeof(int));
        c->is_date_time = realloc(c->is_date_time, (c->columns+1) * sizeof(int));
        produce_column_header(c, s, len);
    } else if (c->rows >= 1 && c->handle.value && c->output_module->csv_value) {
        c->output_module->csv_value(c, s, len);
    }
    if (c->rows >= 1 && c->pass == 1) {
        size_t w = c->column_width[c->columns];
        c->column_width[c->columns] = (len>w) ? len : w;
    }
    c->open_row = 1;
    c->columns++;
}

static void csv_metadata_row(int cc, void *data)
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

readstat_error_t readstat_parse_csv(readstat_parser_t *parser,
        const char *path, struct csv_metadata* md, void *user_ctx) {
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
    md->user_ctx = user_ctx;
    md->handle = parser->handlers;

    rs_read_module_t modules[3] = { rs_read_mod_csv, rs_read_mod_dta, rs_read_mod_sav };
    if ((md->output_module = rs_read_module_for_filename(modules, 3, md->output_format)) == NULL) {
        fprintf(stderr, "Unsupported file format\n");
        retval = READSTAT_ERROR_WRITE;
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
    if (md->handle.metadata && md->pass == 1) {
        readstat_metadata_t metadata = {
            .row_count = md->rows,
            .var_count = md->_columns
        };
        if (md->handle.metadata(&metadata, user_ctx) == READSTAT_HANDLER_ABORT) {
            retval = READSTAT_ERROR_TOO_FEW_COLUMNS;
        }
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
    if (md->is_date_time) {
        free(md->is_date_time);
        md->is_date_time = NULL;
    }
    csv_free(p);
    io->close(io->io_ctx);
    return retval;
}
