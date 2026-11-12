#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../readstat.h"
#include "json_metadata.h"
#include "read_module.h"
#include "csv_metadata.h"
#include "value.h"
#include "../util/file_format.h"
#include "../util/readstat_dta_days.h"

/* SAS dates count days, and SAS datetimes seconds, from 1960-01-01, the
 * same epoch as Stata. Unlike Stata's %tC, SAS datetimes do not count leap
 * seconds. A sas7bdat file cannot hold value labels (SAS keeps them in a
 * separate format catalog), so categories are not written. */

/* Each special missing value takes a lo/hi pair in missing_ranges */
#define SAS_MAX_MISSING_TAGS \
    (sizeof(((readstat_missingness_t *)0)->missing_ranges) / sizeof(readstat_value_t) / 2)

void produce_column_header_sas7bdat(void *csv_metadata, const char *column, readstat_variable_t* var);
void produce_missingness_sas7bdat(void *csv_metadata, const char* column);
void produce_value_label_sas7bdat(void *csv_metadata, const char* column);
void produce_csv_value_sas7bdat(void *csv_metadata, const char *s, size_t len);

rs_read_module_t rs_read_mod_sas7bdat = {
    .format = RS_FORMAT_SAS_DATA,
    .header = &produce_column_header_sas7bdat,
    .missingness = &produce_missingness_sas7bdat,
    .value_label = &produce_value_label_sas7bdat,
    .csv_value = &produce_csv_value_sas7bdat };

static int is_time_sas7bdat(const readstat_variable_t *var) {
    return strncmp(var->format, "TIME", 4) == 0;
}

static double sas_days(const char *s) {
    char *dest;
    int days = readstat_dta_num_days(s, &dest);
    if (dest == s) {
        fprintf(stderr, "%s:%d not a valid date: %s (expected format: yyyy-mm-dd)\n", __FILE__, __LINE__, s);
        exit(EXIT_FAILURE);
    }
    return days;
}

static double sas_seconds_since_midnight(const char *s) {
    int hour, minute;
    double second = 0.0;
    int matched = sscanf(s, "%d:%d:%lf", &hour, &minute, &second);
    if (matched < 2) {
        fprintf(stderr, "%s:%d not a valid time: %s (expected format: hh:MM:SS with optional fractional seconds)\n", __FILE__, __LINE__, s);
        exit(EXIT_FAILURE);
    }
    return hour * 3600.0 + minute * 60.0 + second;
}

static double sas_seconds_since_epoch(const char *s) {
    int year, month, day, hour, minute;
    double second = 0.0;
    int matched = sscanf(s, "%d-%d-%d%*[ T]%d:%d:%lf", &year, &month, &day, &hour, &minute, &second);
    if (matched < 5) {
        fprintf(stderr, "%s:%d not a valid date-time: %s (expected format: yyyy-mm-dd hh:MM:SS with optional fractional seconds)\n", __FILE__, __LINE__, s);
        exit(EXIT_FAILURE);
    }
    char date[32];
    snprintf(date, sizeof(date), "%04d-%02d-%02d", year, month, day);
    return 86400.0 * sas_days(date) + hour * 3600.0 + minute * 60.0 + second;
}

/* Converts a CSV value or a JSON missing value to what SAS stores */
static double sas_double(const char *s, const readstat_variable_t *var, int is_date, int is_date_time) {
    if (is_date)
        return sas_days(s);
    if (is_date_time)
        return sas_seconds_since_epoch(s);
    if (is_time_sas7bdat(var))
        return sas_seconds_since_midnight(s);

    char *dest;
    double val = strtod(s, &dest);
    if (dest == s) {
        fprintf(stderr, "%s:%d not a number: %s\n", __FILE__, __LINE__, s);
        exit(EXIT_FAILURE);
    }
    return val;
}

static double sas_double_from_token(struct csv_metadata *c, jsmntok_t* token) {
    char buf[255];
    int len = token->end - token->start;
    snprintf(buf, sizeof(buf), "%.*s", len, c->json_md->js + token->start);
    return sas_double(buf, &c->variables[c->columns],
            c->is_date[c->columns], c->is_date_time[c->columns]);
}

/* Values from lo to hi are written as the next special missing value: .A, .B, ... */
static void sas_add_missing_range(readstat_variable_t* var, const char *column, double lo, double hi) {
    int idx = var->missingness.missing_ranges_count;
    if (idx >= (int)SAS_MAX_MISSING_TAGS) {
        fprintf(stderr, "%s:%d column %s has more than %d missing value definitions\n",
                __FILE__, __LINE__, column, (int)SAS_MAX_MISSING_TAGS);
        exit(EXIT_FAILURE);
    }
    readstat_value_t value = {
        .type = READSTAT_TYPE_DOUBLE,
        .is_tagged_missing = 1,
        .tag = 'A' + idx
    };
    value.v.double_value = lo;
    var->missingness.missing_ranges[(idx*2)] = value;
    value.v.double_value = hi;
    var->missingness.missing_ranges[(idx*2)+1] = value;
    var->missingness.missing_ranges_count++;
}

static void produce_missingness_discrete_sas7bdat(struct csv_metadata *c, jsmntok_t* missing, const char* column) {
    readstat_variable_t* var = &c->variables[c->columns];
    const char *js = c->json_md->js;

    jsmntok_t* values = find_object_property(js, missing, "values");
    if (!values) {
        fprintf(stderr, "%s:%d Expected to find missing 'values' property\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    int j = 1;
    for (int i=0; i<values->size; i++) {
        jsmntok_t* missing_value_token = values + j;
        if (var->type == READSTAT_TYPE_DOUBLE) {
            double v = sas_double_from_token(c, missing_value_token);
            sas_add_missing_range(var, column, v, v);
        }
        j += slurp_object(missing_value_token);
    }
}

static void produce_missingness_range_sas7bdat(struct csv_metadata *c, jsmntok_t* missing, const char* column) {
    readstat_variable_t* var = &c->variables[c->columns];
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
        sas_add_missing_range(var, column, sas_double_from_token(c, low), sas_double_from_token(c, high));
    }
    if (discrete) {
        double v = sas_double_from_token(c, discrete);
        sas_add_missing_range(var, column, v, v);
    }
}

void produce_missingness_sas7bdat(void *csv_metadata, const char* column) {
    struct csv_metadata *c = (struct csv_metadata *)csv_metadata;
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
        produce_missingness_discrete_sas7bdat(c, missing, column);
    } else if (match_token(js, missing_type, "RANGE")) {
        produce_missingness_range_sas7bdat(c, missing, column);
    } else {
        fprintf(stderr, "%s:%d unknown missing type %.*s\n", __FILE__, __LINE__, missing_type->end - missing_type->start, js+missing_type->start);
        exit(EXIT_FAILURE);
    }
}

void produce_column_header_sas7bdat(void *csv_metadata, const char *column, readstat_variable_t* var) {
    struct csv_metadata *c = (struct csv_metadata *)csv_metadata;
    extract_metadata_type_t coltype = column_type(c->json_md, column, c->output_format);
    if (coltype == EXTRACT_METADATA_TYPE_NUMERIC) {
        /* SAS stores every number as a double */
        var->type = READSTAT_TYPE_DOUBLE;
        extract_metadata_format_t colformat = column_format(c->json_md, column);
        switch (colformat) {
        case EXTRACT_METADATA_FORMAT_DATE:
            snprintf(var->format, sizeof(var->format), "%s", "DATE9.");
        break;
        case EXTRACT_METADATA_FORMAT_TIME:
            snprintf(var->format, sizeof(var->format), "%s", "TIME8.");
        break;
        case EXTRACT_METADATA_FORMAT_DATE_TIME:
            snprintf(var->format, sizeof(var->format), "%s", "DATETIME19.");
        break;
        default:;
            /* PERCENT and CURRENCY values are written as plain numbers:
             * SAS's PERCENTw.d would multiply them by 100 for display */
            int decimals = get_decimals(c->json_md, column);
            if (decimals > 0) {
                snprintf(var->format, sizeof(var->format), "F%d.%d", 10 + decimals, decimals);
            } else {
                snprintf(var->format, sizeof(var->format), "%s", "BEST12.");
            }
        }
    } else if (coltype == EXTRACT_METADATA_TYPE_STRING) {
        var->type = READSTAT_TYPE_STRING;
    }
}

void produce_value_label_sas7bdat(void *csv_metadata, const char* column) {
    struct csv_metadata *c = (struct csv_metadata *)csv_metadata;
    jsmntok_t* categories = find_variable_property(c->json_md->js, c->json_md->tok, column, "categories");
    if (categories) {
        fprintf(stderr, "Warning: value labels for column %s are not written; "
                "a sas7bdat file keeps value labels in a separate SAS catalog\n", column);
    }
}

static readstat_value_t value_double_sas7bdat(const char *s, size_t len, struct csv_metadata *c) {
    readstat_variable_t *var = &c->variables[c->columns];
    double val = sas_double(s, var, c->is_date[c->columns], c->is_date_time[c->columns]);

    readstat_value_t value = {
        .type = READSTAT_TYPE_DOUBLE,
        .v = { .double_value = val }
    };
    int missing_ranges_count = readstat_variable_get_missing_ranges_count(var);
    for (int i=0; i<missing_ranges_count; i++) {
        /* The bounds are tagged, so readstat_double_value() would return NaN */
        readstat_value_t lo_val = readstat_variable_get_missing_range_lo(var, i);
        readstat_value_t hi_val = readstat_variable_get_missing_range_hi(var, i);
        if (val >= lo_val.v.double_value && val <= hi_val.v.double_value) {
            value.is_tagged_missing = 1;
            value.tag = lo_val.tag;
            break;
        }
    }
    return value;
}

void produce_csv_value_sas7bdat(void *csv_metadata, const char *s, size_t len) {
    struct csv_metadata *c = (struct csv_metadata *)csv_metadata;
    readstat_variable_t *var = &c->variables[c->columns];
    int obs_index = c->rows - 1;
    readstat_value_t value;

    if (len == 0) {
        value = value_sysmiss(s, len, c);
    } else if (var->type == READSTAT_TYPE_DOUBLE) {
        value = value_double_sas7bdat(s, len, c);
    } else if (var->type == READSTAT_TYPE_STRING) {
        value = value_string(s, len, c);
    } else {
        fprintf(stderr, "%s:%d unsupported variable type %d\n", __FILE__, __LINE__, var->type);
        exit(EXIT_FAILURE);
    }

    c->handle.value(obs_index, var, value, c->user_ctx);
}
