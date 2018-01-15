#include <stdio.h>
#include <stdlib.h>

#include "../../readstat.h"
#include "../format.h"
#include "json_metadata.h"

#include "../util/readstat_sav_date.h"
#include "produce_csv_value.h"
#include "produce_csv_column_header.h"
#include "produce_missingness_sav.h"

double get_double_date_missing_sav(const char *js, jsmntok_t* missing_value_token) {
    // SAV missing date
    char buf[255];
    char *dest;
    int len = missing_value_token->end - missing_value_token->start;
    snprintf(buf, sizeof(buf), "%.*s", len, js + missing_value_token->start);
    double val = readstat_sav_date_parse(buf, &dest);
    if (buf == dest) {
        fprintf(stderr, "%s:%d failed to parse double: %s\n", __FILE__, __LINE__, buf);
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "added double date missing %s\n", buf);
    }
    return val;
}

void produce_missingness_discrete_sav(struct csv_metadata *c, jsmntok_t* missing, const char* column) {
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
            readstat_variable_add_missing_double_value(var, get_double_date_missing_sav(js, missing_value_token));
        } else if (var->type == READSTAT_TYPE_DOUBLE) {
            readstat_variable_add_missing_double_value(var, get_double_from_token(js, missing_value_token));
        } else if (var->type == READSTAT_TYPE_STRING) {
        } else {
            fprintf(stderr, "%s:%d Unsupported column type %d\n", __FILE__, __LINE__, var->type);
            exit(EXIT_FAILURE);
        }
        j += slurp_object(missing_value_token);
    }
}

void produce_missingness_range_sav(struct csv_metadata *c, jsmntok_t* missing, const char* column) {
    readstat_variable_t* var = &c->variables[c->columns];
    int is_date = c->is_date[c->columns];
    const char *js = c->json_md->js;

    jsmntok_t* low = find_object_property(js, missing, "low");
    jsmntok_t* high = find_object_property(js, missing, "high");
    jsmntok_t* discrete = find_object_property(js, missing, "discrete-value");

    if (low && !high) {
        fprintf(stderr, "%s:%d missing.low specified for column %s, but missing.high not specified\n", __FILE__, __LINE__, column);
        exit(EXIT_FAILURE);
    }
    if (high && !low) {
        fprintf(stderr, "%s:%d missing.high specified for column %s, but missing.low not specified\n", __FILE__, __LINE__, column);
        exit(EXIT_FAILURE);
    }

    if (low && high) {
        double lo = is_date ? get_double_date_missing_sav(js, low) : get_double_from_token(js, low);
        double hi = is_date ? get_double_date_missing_sav(js, high) :  get_double_from_token(js, high);
        readstat_variable_add_missing_double_range(var, lo, hi);
    }

    if (discrete) {
        double v = is_date ? get_double_date_missing_sav(js, discrete) : get_double_from_token(js, discrete);
        readstat_variable_add_missing_double_value(var, v);
    }
}

void produce_missingness_sav(struct csv_metadata *c, const char* column) {
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
        produce_missingness_discrete_sav(c, missing, column);
    } else if (match_token(js, missing_type, "RANGE")) {
        produce_missingness_range_sav(c, missing, column);
    } else {
        fprintf(stderr, "%s:%d unknown missing type %.*s\n", __FILE__, __LINE__, missing_type->end - missing_type->start, js+missing_type->start);
        exit(EXIT_FAILURE);
    }
}
