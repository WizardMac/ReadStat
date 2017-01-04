#include <stdio.h>
#include <stdlib.h>

#include "../../readstat.h"
#include "../format.h"
#include "json_metadata.h"

#include "produce_value_label.h"
#include "produce_missingness.h"
#include "produce_csv_column_header.h"

void produce_column_header_csv(char *column, readstat_variable_t* var, struct csv_metadata *c) {
    metadata_column_type_t coltype = column_type(c->json_md, column, c->output_format);
    if (coltype == METADATA_COLUMN_TYPE_DATE) {
        var->type = READSTAT_TYPE_STRING;
    } else if (coltype == METADATA_COLUMN_TYPE_NUMERIC) {
        var->type = READSTAT_TYPE_DOUBLE;
    } else if (coltype == METADATA_COLUMN_TYPE_STRING) {
        var->type = READSTAT_TYPE_STRING;
    }
}

void produce_column_header_dta(char *column, readstat_variable_t* var, struct csv_metadata *c) {
    metadata_column_type_t coltype = column_type(c->json_md, column, c->output_format);
    if (coltype == METADATA_COLUMN_TYPE_DATE) {
        snprintf(var->format, sizeof(var->format), "%s", "%td");
        var->type = READSTAT_TYPE_INT32;
    } else if (coltype == METADATA_COLUMN_TYPE_NUMERIC) {
        var->type = READSTAT_TYPE_DOUBLE;
        snprintf(var->format, sizeof(var->format), "%%9.%df", get_decimals(c->json_md, column));
    } else if (coltype == METADATA_COLUMN_TYPE_STRING) {
        var->type = READSTAT_TYPE_STRING;
    }
}

void produce_column_header_sav(char *column, readstat_variable_t* var, struct csv_metadata *c) {
    metadata_column_type_t coltype = column_type(c->json_md, column, c->output_format);
    if (coltype == METADATA_COLUMN_TYPE_DATE) {
        var->type = READSTAT_TYPE_DOUBLE;
        snprintf(var->format, sizeof(var->format), "%s", "EDATE40");
    } else if (coltype == METADATA_COLUMN_TYPE_NUMERIC) {
        var->type = READSTAT_TYPE_DOUBLE;
        snprintf(var->format, sizeof(var->format), "F8.%d", get_decimals(c->json_md, column));
    } else if (coltype == METADATA_COLUMN_TYPE_STRING) {
        var->type = READSTAT_TYPE_STRING;
    }
}

void produce_column_header(void *s, size_t len, void *data) {
    struct csv_metadata *c = (struct csv_metadata *)data;
    char* column = (char*)s;
    readstat_variable_t* var = &c->variables[c->columns];
    memset(var, 0, sizeof(readstat_variable_t));
    metadata_column_type_t coltype = column_type(c->json_md, column, c->output_format);
    c->is_date[c->columns] = coltype == METADATA_COLUMN_TYPE_DATE;

    
    if (coltype == METADATA_COLUMN_TYPE_STRING) {
        var->alignment = READSTAT_ALIGNMENT_LEFT;
    } else if (coltype == METADATA_COLUMN_TYPE_NUMERIC || coltype == METADATA_COLUMN_TYPE_DATE) {
        var->alignment = READSTAT_ALIGNMENT_RIGHT;
    }

    if (c->output_format == RS_FORMAT_CSV) {
        produce_column_header_csv(column, var, c);
    } else if (c->output_format == RS_FORMAT_DTA) {
        produce_column_header_dta(column, var, c);
    } else if (c->output_format == RS_FORMAT_SAV) {
        produce_column_header_sav(column, var, c);
    } else {
        fprintf(stderr, "%s:%d unsupported output format %d\n", __FILE__, __LINE__, c->output_format);
        exit(EXIT_FAILURE);
    }

    if (c->pass == 2 && coltype == METADATA_COLUMN_TYPE_STRING) {
        var->storage_width = c->column_width[c->columns];
    }
    
    var->index = c->columns;
    copy_variable_property(c->json_md, column, "label", var->label, sizeof(var->label));
    snprintf(var->name, sizeof(var->name), "%.*s", (int)len, column);

    produce_missingness(c, column);
    if (c->parser->value_label_handler) {
        produce_value_label(c, column);
    }

    if (c->parser->variable_handler && c->pass == 2) {
        c->parser->variable_handler(c->columns, var, column, c->user_ctx);
    }
}
