#include <stdlib.h>
#include <stdio.h>

#include "../../readstat.h"
#include "json_metadata.h"
#include "read_module.h"
#include "csv_metadata.h"
#include "value.h"
#include "../util/file_format.h"
#include "../util/readstat_dta_days.h"

void produce_column_header_dta(void *csv_metadata, const char *column, readstat_variable_t* var);
void produce_missingness_dta(void *csv_metadata, const char* column);
void produce_value_label_dta(void *csv_metadata, const char* column);
void produce_csv_value_dta(void *csv_metadata, const char *s, size_t len);

rs_read_module_t rs_read_mod_dta = {
    .format = RS_FORMAT_DTA,
    .header = &produce_column_header_dta,
    .missingness = &produce_missingness_dta,
    .value_label = &produce_value_label_dta,
    .csv_value = &produce_csv_value_dta };

static double get_dta_days_from_token(const char *js, jsmntok_t* token) {
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

static char dta_add_missing_date(readstat_variable_t* var, double v) {
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

static char dta_add_missing_double(readstat_variable_t* var, double v) {
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

static void produce_missingness_range_dta(struct csv_metadata *c, jsmntok_t* missing, const char* column) {
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

static void produce_missingness_discrete_dta(struct csv_metadata *c, jsmntok_t* missing, const char* column) {
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


void produce_missingness_dta(void *csv_metadata, const char* column) {
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
        produce_missingness_discrete_dta(c, missing, column);
    } else if (match_token(js, missing_type, "RANGE")) {
        produce_missingness_range_dta(c, missing, column);
    } else {
        fprintf(stderr, "%s:%d unknown missing type %.*s\n", __FILE__, __LINE__, missing_type->end - missing_type->start, js+missing_type->start);
        exit(EXIT_FAILURE);
    }
}

void produce_column_header_dta(void *csv_metadata, const char *column, readstat_variable_t* var) {
    struct csv_metadata *c = (struct csv_metadata *)csv_metadata;
    extract_metadata_type_t coltype = column_type(c->json_md, column, c->output_format);
    if (coltype == EXTRACT_METADATA_TYPE_NUMERIC) {
        extract_metadata_format_t colformat = column_format(c->json_md, column);
        switch (colformat) {
        case EXTRACT_METADATA_FORMAT_NUMBER:
        case EXTRACT_METADATA_FORMAT_PERCENT:
        case EXTRACT_METADATA_FORMAT_CURRENCY:
            var->type = READSTAT_TYPE_DOUBLE;
            snprintf(var->format, sizeof(var->format), "%%9.%df", get_decimals(c->json_md, column));
        break;
        case EXTRACT_METADATA_FORMAT_DATE:
            var->type = READSTAT_TYPE_INT32;
            snprintf(var->format, sizeof(var->format), "%s", "%td");
        break;
        case EXTRACT_METADATA_FORMAT_TIME:
        case EXTRACT_METADATA_FORMAT_DATE_TIME:
            var->type = READSTAT_TYPE_DOUBLE;
            snprintf(var->format, sizeof(var->format), "%s", "%tC");
            // %tC => is equivalent to coordinated universal time (UTC)
        break;
        default:
            var->type = READSTAT_TYPE_DOUBLE;
            snprintf(var->format, sizeof(var->format), "%%9.%df", get_decimals(c->json_md, column));
        }
    } else if (coltype == EXTRACT_METADATA_TYPE_STRING) {
        var->type = READSTAT_TYPE_STRING;
    }
}

static void produce_value_label_int32_date_dta(const char* column, struct csv_metadata *c, char *code, char *label) {
    readstat_variable_t* variable = &c->variables[c->columns];
    char *dest;
    int days = readstat_dta_num_days(code, &dest);
    if (dest == code) {
        fprintf(stderr, "%s:%d not a valid date: %s\n", __FILE__, __LINE__, code);
        exit(EXIT_FAILURE);
    }
    readstat_value_t value = {
        .v = { .i32_value = days },
        .type = READSTAT_TYPE_INT32,
    };

    int missing_ranges_count = readstat_variable_get_missing_ranges_count(variable);
    for (int i=0; i<missing_ranges_count; i++) {
        readstat_value_t lo_val = readstat_variable_get_missing_range_lo(variable, i);
        readstat_value_t hi_val = readstat_variable_get_missing_range_hi(variable, i);
        if (readstat_value_type(lo_val) == READSTAT_TYPE_INT32) {
            int32_t lo = readstat_int32_value(lo_val);
            int32_t hi = readstat_int32_value(hi_val);
            if (days >= lo && days <= hi) {
                value.is_tagged_missing = 1;
                value.tag = 'a' + i;
            }
        }
    }
    c->handle.value_label(column, value, label, c->user_ctx);
}

static void produce_value_label_double_dta(const char* column, struct csv_metadata *c, const char *code, const char *label) {
    readstat_variable_t* variable = &c->variables[c->columns];
    char *endptr;
    double v = strtod(code, &endptr);
    if (endptr == code) {
        fprintf(stderr, "%s:%d not a number: %s\n", __FILE__, __LINE__, code);
        exit(EXIT_FAILURE);
    }
    readstat_value_t value = {
        .v = { .double_value = v },
        .type = READSTAT_TYPE_DOUBLE,
    };
    int missing_ranges_count = readstat_variable_get_missing_ranges_count(variable);
    for (int i=0; i<missing_ranges_count; i++) {
        readstat_value_t lo_val = readstat_variable_get_missing_range_lo(variable, i);
        readstat_value_t hi_val = readstat_variable_get_missing_range_hi(variable, i);
        if (readstat_value_type(lo_val) == READSTAT_TYPE_DOUBLE) {
            double lo = readstat_double_value(lo_val);
            double hi = readstat_double_value(hi_val);
            if (v >= lo && v <= hi) {
                value.is_tagged_missing = 1;
                value.tag = 'a' + i;
            }
        }
    }
    c->handle.value_label(column, value, label, c->user_ctx);
}

void produce_value_label_dta(void *csv_metadata, const char* column) {
    struct csv_metadata *c = (struct csv_metadata *)csv_metadata;
    jsmntok_t* categories = find_variable_property(c->json_md->js, c->json_md->tok, column, "categories");
    if (categories==NULL) {
        return;
    }
    readstat_variable_t* variable = &c->variables[c->columns];
    readstat_type_t coltype = variable->type;

    int is_date = c->is_date[c->columns];
    int j = 1;
    char code_buf[1024];
    char label_buf[1024];
    for (int i=0; i<categories->size; i++) {
        jsmntok_t* tok = categories+j;
        char* code = get_object_property(c->json_md->js, tok, "code", code_buf, sizeof(code_buf));
        char* label = get_object_property(c->json_md->js, tok, "label", label_buf, sizeof(label_buf));
        if (!code || !label) {
            fprintf(stderr, "%s:%d bogus JSON metadata input. Missing code/label for column %s\n", __FILE__, __LINE__, column);
            exit(EXIT_FAILURE);
        }

        if (is_date) {
            produce_value_label_int32_date_dta(column, c, code, label);
        } else if (coltype == READSTAT_TYPE_DOUBLE) {
            produce_value_label_double_dta(column, c, code, label);
        } else if (coltype == READSTAT_TYPE_STRING) {
        } else {
            fprintf(stderr, "%s:%d unsupported column type %d for value label for column %s\n", __FILE__, __LINE__, coltype, column);
            exit(EXIT_FAILURE);
        }
        j += slurp_object(tok);
    }
}

static readstat_value_t value_int32_date_dta(const char *s, size_t len, struct csv_metadata *c) {
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

static readstat_value_t value_double_dta(const char *s, size_t len, struct csv_metadata *c) {
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

static readstat_value_t value_double_date_time_dta(const char *s, size_t len, struct csv_metadata *c) {
    // Handle empty or NULL strings as missing values
    if (s == NULL || len == 0 || *s == '\0') {
        readstat_value_t value = {
            .type = READSTAT_TYPE_DOUBLE,
            .is_system_missing = 1,
            .v = { .double_value = NAN }
        };
        return value;
    }

    // Truncate the date string to 23 characters to remove the timezone offset and
    // microseconds, if present. STATA does not support timezones or microseconds.
    char date_time[24];
    snprintf(date_time, sizeof(date_time), "%s", s);

    // Parse date-time components
    int year, month, day, hour, minute, second, msecs = 0;
    int matched = sscanf(
        date_time,
        "%d-%d-%d %d:%d:%d.%d",
        &year, &month, &day, &hour, &minute, &second, &msecs
    );
    if (matched < 6 || matched > 8) {
        fprintf(stderr, "%s:%d not a valid date-time: %s (expected format: yyyy-mm-dd hh:MM:SS with optional milliseconds. Datetime string is truncated at 23 characters to ignore microseconds and timezone information.)\n", __FILE__, __LINE__, date_time);
        exit(EXIT_FAILURE);
    }

    // Get days since the epoch for the date
    char days_since_epoch_string[11];
    snprintf(days_since_epoch_string, sizeof(days_since_epoch_string), "%04d-%02d-%02d", year, month, day);
    char* dest;
    int days_since_epoch = readstat_dta_num_days(days_since_epoch_string, &dest);

    // Add the hours, minutes, and seconds to the days
    double msecs_since_epoch = 86400000.0 * days_since_epoch + hour * 3600000.0 + minute * 60000.0 + second * 1000.0 + msecs * 1.0;

    // Adjust for leap seconds; 27 have occurred as of writing this code
    // https://en.m.wikipedia.org/wiki/Leap_second
    typedef struct {
        int year;
        int month;
        int day;
    } leap_second_date;
    
    leap_second_date leap_seconds[] = {
        {1972, 6, 30}, {1972, 12, 31},  // +2 seconds in 1972
        {1973, 12, 31},                 // +1 second in 1973
        {1974, 12, 31},                 // +1 second in 1974
        {1975, 12, 31},                 // +1 second in 1975
        {1976, 12, 31},                 // +1 second in 1976
        {1977, 12, 31},                 // +1 second in 1977
        {1978, 12, 31},                 // +1 second in 1978
        {1979, 12, 31},                 // +1 second in 1979
        {1981, 6, 30},                  // +1 second in 1981
        {1982, 6, 30},                  // +1 second in 1982
        {1983, 6, 30},                  // +1 second in 1983
        {1985, 6, 30},                  // +1 second in 1985
        {1987, 12, 31},                 // +1 second in 1987
        {1989, 12, 31},                 // +1 second in 1989
        {1990, 12, 31},                 // +1 second in 1990
        {1992, 6, 30},                  // +1 second in 1992
        {1993, 6, 30},                  // +1 second in 1993
        {1994, 6, 30},                  // +1 second in 1994
        {1995, 12, 31},                 // +1 second in 1995
        {1997, 6, 30},                  // +1 second in 1997
        {1998, 12, 31},                 // +1 second in 1998
        {2005, 12, 31},                 // +1 second in 2005
        {2008, 12, 31},                 // +1 second in 2008
        {2012, 6, 30},                  // +1 second in 2012
        {2015, 6, 30},                  // +1 second in 2015
        {2016, 12, 31}                  // +1 second in 2016
    };

    int leap_second_count = sizeof(leap_seconds) / sizeof(leap_seconds[0]);
    int leap_seconds_to_add = 0;

    for (int i = 0; i < leap_second_count; i++) {
        // If the date is after this leap second, add one second
        if (
            (year > leap_seconds[i].year) ||
            (year == leap_seconds[i].year && month > leap_seconds[i].month) ||
            (year == leap_seconds[i].year && month == leap_seconds[i].month && day > leap_seconds[i].day)
        ) { leap_seconds_to_add++; }
    }
    msecs_since_epoch += leap_seconds_to_add * 1000.0;

    readstat_value_t value = {
        .type = READSTAT_TYPE_DOUBLE,
        .v = { .double_value = msecs_since_epoch }
    };

    return value;
}

void produce_csv_value_dta(void *csv_metadata, const char *s, size_t len) {
    struct csv_metadata *c = (struct csv_metadata *)csv_metadata;
    readstat_variable_t *var = &c->variables[c->columns];
    int is_date = c->is_date[c->columns];
    int is_date_time = c->is_date_time[c->columns];
    int obs_index = c->rows - 1; // TODO: ???
    readstat_value_t value;

    if (len == 0) {
        value = value_sysmiss(s, len, c);
    } else if (is_date) {
        value = value_int32_date_dta(s, len, c);
    } else if (is_date_time) {
        value = value_double_date_time_dta(s, len, c);
    } else if (var->type == READSTAT_TYPE_DOUBLE) {
        value = value_double_dta(s, len, c);
    } else if (var->type == READSTAT_TYPE_STRING) {
        value = value_string(s, len, c);
    } else {
        fprintf(stderr, "%s:%d unsupported variable type %d\n", __FILE__, __LINE__, var->type);
        exit(EXIT_FAILURE);
    }

    c->handle.value(obs_index, var, value, c->user_ctx);
}
