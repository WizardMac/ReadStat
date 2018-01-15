#include <stdio.h>
#include <stdlib.h>

#include "../../readstat.h"
#include "../format.h"
#include "produce_csv_column_header.h"
#include "produce_csv_value.h"
#include "produce_csv_value_csv.h"
#include "produce_csv_value_dta.h"
#include "produce_csv_value_sav.h"

void produce_csv_column_value(void *s, size_t len, void *data) {
    struct csv_metadata *c = (struct csv_metadata *)data;
    const char *ss = (const char*) s;
    if (c->output_format == RS_FORMAT_CSV) {
        produce_csv_value_csv(ss, len, c);
    } else if (c->output_format == RS_FORMAT_DTA) {
        produce_csv_value_dta(ss, len, c);
    } else if (c->output_format == RS_FORMAT_SAV) {
        produce_csv_value_sav(ss, len, c);
    } else {
        fprintf(stderr, "%s:%d unsupported output format %d\n", __FILE__, __LINE__, c->output_format);
        exit(EXIT_FAILURE);
    }
}

readstat_value_t value_sysmiss(const char *s, size_t len, struct csv_metadata *c) {
    readstat_variable_t *var = &c->variables[c->columns];
    readstat_value_t value = {
        .is_system_missing = 1,
        .is_tagged_missing = 0,
        .type = var->type
    };
    return value;
}

readstat_value_t value_string(const char *s, size_t len, struct csv_metadata *c) {
    readstat_value_t value = {
            .is_system_missing = 0,
            .is_tagged_missing = 0,
            .v = { .string_value = s },
            .type = READSTAT_TYPE_STRING
        };
    return value;
}

readstat_value_t value_double(const char *s, size_t len, struct csv_metadata *c) {
    char *dest;
    double val = strtod(s, &dest);
    if (dest == s) {
        fprintf(stderr, "%s:%d not a number: %s\n", __FILE__, __LINE__, (char*)s);
        exit(EXIT_FAILURE);
    }
    readstat_value_t value = {
        .type = READSTAT_TYPE_DOUBLE,
        .v = { .double_value = val }
    };
    return value;
}
