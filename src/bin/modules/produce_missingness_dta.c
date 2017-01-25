#include <stdio.h>
#include <stdlib.h>

#include "../../readstat.h"
#include "../format.h"
#include "json_metadata.h"

#include "../util/readstat_dta_days.h"
#include "produce_csv_value.h"
#include "produce_csv_column_header.h"
#include "produce_missingness_dta.h"

double get_dta_days_from_token(const char *js, jsmntok_t* token) {
    char buf[255];
    int len = token->end - token->start;
    snprintf(buf, sizeof(buf), "%.*s", len, js + token->start);
    char* dest;
    int days = readstat_dta_num_days(buf, &dest);
    if (dest == buf) {
        fprintf(stderr, "%s:%d error parsing date %s\n", __FILE__, __LINE__, buf);
        exit(EXIT_FAILURE);
    }
    return days;
}

char dta_add_missing_date(readstat_variable_t* var, double v) {
    int idx = var->missingness.missing_ranges_count;
    char tagg = 'a' + idx;
    if (tagg > 'z') {
        fprintf(stderr, "%s:%d missing tag reached %c, aborting ...\n", __FILE__, __LINE__, tagg);
        exit(EXIT_FAILURE);
    }
    readstat_value_t value = {
        .type = READSTAT_TYPE_INT32,
        .is_system_missing = 0,
        .is_tagged_missing = 1,
        .tag = tagg,
        .v = {
            .i32_value = v
        }
    };
    var->missingness.missing_ranges[(idx*2)] = value;
    var->missingness.missing_ranges[(idx*2)+1] = value;
    var->missingness.missing_ranges_count++;
    return tagg;
}

char dta_add_missing_double(readstat_variable_t* var, double v) {
    int idx = var->missingness.missing_ranges_count;
    char tagg = 'a' + idx;
    if (tagg > 'z') {
        fprintf(stderr, "%s:%d missing tag reached %c, aborting ...\n", __FILE__, __LINE__, tagg);
        exit(EXIT_FAILURE);
    }
    readstat_value_t value = {
        .type = READSTAT_TYPE_DOUBLE,
        .is_system_missing = 0,
        .is_tagged_missing = 1,
        .tag = tagg,
        .v = {
            .double_value = v
        }
    };
    var->missingness.missing_ranges[(idx*2)] = value;
    var->missingness.missing_ranges[(idx*2)+1] = value;
    var->missingness.missing_ranges_count++;
    return tagg;
}

void produce_missingness_range_dta(struct csv_metadata *c, jsmntok_t* missing, const char* column) {
    readstat_variable_t* var = &c->variables[c->columns];
    const char *js = c->json_md->js;
    int is_date = c->is_date[c->columns];

    jsmntok_t* low = find_object_property(js, missing, "low");
    jsmntok_t* high = find_object_property(js, missing, "high");
    jsmntok_t* discrete = find_object_property(js, missing, "discrete-value");

    jsmntok_t* categories = find_variable_property(js, c->json_md->tok, column, "categories");
    if (!categories && (low || high || discrete)) {
        fprintf(stderr, "%s:%d expected to find categories for column %s\n", __FILE__, __LINE__, column);
        exit(EXIT_FAILURE);
    } else if (!categories) {
        return;
    }
    if (low && !high) {
        fprintf(stderr, "%s:%d missing.low specified for column %s, but missing.high not specified\n", __FILE__, __LINE__, column);
        exit(EXIT_FAILURE);
    }
    if (high && !low) {
        fprintf(stderr, "%s:%d missing.high specified for column %s, but missing.low not specified\n", __FILE__, __LINE__, column);
        exit(EXIT_FAILURE);
    }

    char label_buf[1024];
    int j = 1;
    for (int i=0; i<categories->size; i++) {
        jsmntok_t* tok = categories+j;
        jsmntok_t* code = find_object_property(js, tok, "code");
        char* label = get_object_property(c->json_md->js, tok, "label", label_buf, sizeof(label_buf));
        if (!code || !label) {
            fprintf(stderr, "%s:%d bogus JSON metadata input. Missing code/label for column %s\n", __FILE__, __LINE__, column);
            exit(EXIT_FAILURE);
        }

        double cod = is_date ? get_dta_days_from_token(js, code) : get_double_from_token(js, code);

        if (low && high) {
            double lo = is_date ? get_dta_days_from_token(js, low) : get_double_from_token(js, low);
            double hi = is_date ? get_dta_days_from_token(js, high) : get_double_from_token(js, high);
            if (cod >= lo && cod <= hi) {
                is_date ? dta_add_missing_date(var, cod) : dta_add_missing_double(var, cod);
            }
        }
        if (discrete) {
            double v = is_date ? get_dta_days_from_token(js, discrete) : get_double_from_token(js, discrete);
            if (cod == v) {
                is_date ? dta_add_missing_date(var, cod) : dta_add_missing_double(var, cod);
            }
        }
        j += slurp_object(tok);
    }
}

void produce_missingness_discrete_dta(struct csv_metadata *c, jsmntok_t* missing, const char* column) {
    readstat_variable_t* var = &c->variables[c->columns];
    int is_date = c->is_date[c->columns];
    const char *js = c->json_md->js;

    jsmntok_t* values = find_object_property(js, missing, "values");
    if (!values) {
        fprintf(stderr, "%s:%d Expected to find missing 'values' property\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    int j = 1;
    for (int i=0; i<values->size; i++) {
        jsmntok_t* missing_value_token = values + j;
        if (is_date) { 
            dta_add_missing_date(var, get_dta_days_from_token(js, missing_value_token));
        } else if (var->type == READSTAT_TYPE_DOUBLE) {
            dta_add_missing_double(var, get_double_from_token(js, missing_value_token));
        } else if (var->type == READSTAT_TYPE_STRING) {
        } else {
            fprintf(stderr, "%s:%d Unsupported column type %d\n", __FILE__, __LINE__, var->type);
            exit(EXIT_FAILURE);
        }
        j += slurp_object(missing_value_token);
    }
}


void produce_missingness_dta(struct csv_metadata *c, const char* column) {
    const char *js = c->json_md->js;
    readstat_variable_t* var = &c->variables[c->columns];
    var->missingness.missing_ranges_count = 0;
    
    jsmntok_t* missing = find_variable_property(js, c->json_md->tok, column, "missing");
    if (!missing) {
        return;
    }

    jsmntok_t* missing_type = find_object_property(js, missing, "type");
    if (!missing_type) {
        fprintf(stderr, "%s:%d expected to find missing.type for column %s\n", __FILE__, __LINE__, column);
        exit(EXIT_FAILURE);
    }

    if (match_token(js, missing_type, "DISCRETE")) {
        produce_missingness_discrete_dta(c, missing, column);
    } else if (match_token(js, missing_type, "RANGE")) {
        produce_missingness_range_dta(c, missing, column);
    } else {
        fprintf(stderr, "%s:%d unknown missing type %.*s\n", __FILE__, __LINE__, missing_type->end - missing_type->start, js+missing_type->start);
        exit(EXIT_FAILURE);
    }
}
