#include <stdio.h>
#include <stdlib.h>

#include "../util/readstat_dta_days.h"

#include "produce_csv_value.h"
#include "produce_csv_value_dta.h"
#include "produce_csv_column_header.h"

readstat_value_t value_int32_date_dta(const char *s, size_t len, struct csv_metadata *c) {
    readstat_variable_t *var = &c->variables[c->columns];
    char* dest;
    int val = readstat_dta_num_days(s, &dest);
    if (dest == s) {
        fprintf(stderr, "%s:%d not a date: %s\n", __FILE__, __LINE__, (char*)s);
        exit(EXIT_FAILURE);
    }
    
    int missing_ranges_count = readstat_variable_get_missing_ranges_count(var);
    for (int i=0; i<missing_ranges_count; i++) {
        readstat_value_t lo_val = readstat_variable_get_missing_range_lo(var, i);
        readstat_value_t hi_val = readstat_variable_get_missing_range_hi(var, i);
        if (readstat_value_type(lo_val) != READSTAT_TYPE_INT32) {
            fprintf(stderr, "%s:%d expected type of lo_val to be of type int32. Should not happen\n", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
        int lo = readstat_int32_value(lo_val);
        int hi = readstat_int32_value(hi_val);
        if (val >= lo && val <= hi) {
            readstat_value_t value = {
                .type = READSTAT_TYPE_INT32,
                .is_tagged_missing = 1,
                .tag = 'a' + i,
                .v = { .i32_value = val }
                };
            return value;
        }
    }
    readstat_value_t value = {
        .type = READSTAT_TYPE_INT32,
        .is_tagged_missing = 0,
        .v = { .i32_value = val }
    };
    return value;
}

readstat_value_t value_double_dta(const char *s, size_t len, struct csv_metadata *c) {
    char *dest;
    readstat_variable_t *var = &c->variables[c->columns];
    double val = strtod(s, &dest);
    if (dest == s) {
        fprintf(stderr, "not a number: %s\n", (char*)s);
        exit(EXIT_FAILURE);
    }
    int missing_ranges_count = readstat_variable_get_missing_ranges_count(var);
    for (int i=0; i<missing_ranges_count; i++) {
        readstat_value_t lo_val = readstat_variable_get_missing_range_lo(var, i);
        readstat_value_t hi_val = readstat_variable_get_missing_range_hi(var, i);
        if (readstat_value_type(lo_val) != READSTAT_TYPE_DOUBLE) {
            fprintf(stderr, "%s:%d expected type of lo_val to be of type double. Should not happen\n", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
        double lo = readstat_double_value(lo_val);
        double hi = readstat_double_value(hi_val);
        if (val >= lo && val <= hi) {
            readstat_value_t value = {
                .type = READSTAT_TYPE_DOUBLE,
                .is_tagged_missing = 1,
                .tag = 'a' + i,
                .v = { .double_value = val }
                };
            return value;
        }
    }

    readstat_value_t value = {
        .type = READSTAT_TYPE_DOUBLE,
        .is_tagged_missing = 0,
        .v = { .double_value = val }
    };
    return value;
}

void produce_csv_value_dta(const char *s, size_t len, struct csv_metadata *c) {
    readstat_variable_t *var = &c->variables[c->columns];
    int is_date = c->is_date[c->columns];
    int obs_index = c->rows - 1; // TODO: ???
    readstat_value_t value;

    if (len == 0) {
        value = value_sysmiss(s, len, c);
    } else if (is_date) {
        value = value_int32_date_dta(s, len, c);
    } else if (var->type == READSTAT_TYPE_DOUBLE) {
        value = value_double_dta(s, len, c);
    } else if (var->type == READSTAT_TYPE_STRING) {
        value = value_string(s, len, c);
    } else {
        fprintf(stderr, "%s:%d unsupported variable type %d\n", __FILE__, __LINE__, var->type);
        exit(EXIT_FAILURE);
    }

    c->parser->value_handler(obs_index, var, value, c->user_ctx);
}
