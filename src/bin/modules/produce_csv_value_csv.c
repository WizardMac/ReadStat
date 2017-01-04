#include <stdio.h>
#include <stdlib.h>

#include "produce_csv_value.h"
#include "produce_csv_value_csv.h"
#include "produce_csv_column_header.h"

void produce_csv_value_csv(const char *s, size_t len, struct csv_metadata *c) {
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

    c->parser->value_handler(obs_index, var, value, c->user_ctx);
}
