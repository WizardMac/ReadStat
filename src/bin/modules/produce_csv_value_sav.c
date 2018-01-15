#include <stdio.h>
#include <stdlib.h>

#include "produce_csv_value.h"
#include "produce_csv_value_sav.h"
#include "produce_csv_column_header.h"
#include "../util/readstat_sav_date.h"

readstat_value_t value_double_date_sav(const char *s, size_t len, struct csv_metadata *c) {
    char *dest;
    double val = readstat_sav_date_parse(s, &dest);
    if (dest == s) {
        fprintf(stderr, "%s:%d not a valid date: %s\n", __FILE__, __LINE__, (char*)s);
        exit(EXIT_FAILURE);
    }
    readstat_value_t value = {
        .type = READSTAT_TYPE_DOUBLE,
        .v = { .double_value = val }
    };
    return value;
}

void produce_csv_value_sav(const char *s, size_t len, struct csv_metadata *c) {
    readstat_variable_t *var = &c->variables[c->columns];
    int is_date = c->is_date[c->columns];
    int obs_index = c->rows - 1; // TODO: ???
    readstat_value_t value;

    if (len == 0) {
        value = value_sysmiss(s, len, c);
    } else if (is_date) {
        value = value_double_date_sav(s, len, c);
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
