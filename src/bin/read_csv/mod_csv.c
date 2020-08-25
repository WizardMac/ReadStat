#include <stdlib.h>

#include "../../readstat.h"
#include "json_metadata.h"
#include "read_module.h"
#include "csv_metadata.h"
#include "value.h"
#include "../util/file_format.h"

static void produce_column_header_csv(void *csv_metadata, const char *column, readstat_variable_t* var);
void produce_csv_value_csv(void *csv_metadata, const char *s, size_t len);

rs_read_module_t rs_read_mod_csv = {
    .format = RS_FORMAT_CSV,
    .header = &produce_column_header_csv,
    .csv_value = &produce_csv_value_csv };

static void produce_column_header_csv(void *csv_metadata, const char *column, readstat_variable_t* var) {
    struct csv_metadata *c = (struct csv_metadata *)csv_metadata;
    metadata_column_type_t coltype = column_type(c->json_md, column, c->output_format);
    if (coltype == METADATA_COLUMN_TYPE_DATE) {
        var->type = READSTAT_TYPE_STRING;
    } else if (coltype == METADATA_COLUMN_TYPE_NUMERIC) {
        var->type = READSTAT_TYPE_DOUBLE;
    } else if (coltype == METADATA_COLUMN_TYPE_STRING) {
        var->type = READSTAT_TYPE_STRING;
    }
}

void produce_csv_value_csv(void *csv_metadata, const char *s, size_t len) {
    struct csv_metadata *c = (struct csv_metadata *)csv_metadata;
    readstat_variable_t *var = &c->variables[c->columns];
    int is_date = c->is_date[c->columns];
    int obs_index = c->rows - 1; // TODO: ???
    readstat_value_t value;

    if (len == 0) {
        value = value_sysmiss(s, len, c);
    } else if (is_date) {
        value = value_string(s, len, c);
    } else if (var->type == READSTAT_TYPE_DOUBLE) {
        value = value_double(s, len, c);
    } else if (var->type == READSTAT_TYPE_STRING) {
        value = value_string(s, len, c);
    } else {
        fprintf(stderr, "%s:%d unsupported variable type %d\n", __FILE__, __LINE__, var->type);
        exit(EXIT_FAILURE);
    }

    c->handle.value(obs_index, var, value, c->user_ctx);
}
