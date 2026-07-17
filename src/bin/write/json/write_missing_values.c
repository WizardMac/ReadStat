#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "../../../readstat.h"
#include "../../util/readstat_sav_date.h"
#include "../../util/readstat_dta_days.h"

#include "../../extract_metadata.h"
#include "write_missing_values.h"

static void handle_missing_discrete(struct context *ctx, readstat_variable_t *variable) {
    const char *format = readstat_variable_get_format(variable);
    int spss_date = format && (strcmp(format, "EDATE40") == 0) && variable->type == READSTAT_TYPE_DOUBLE;
    int missing_ranges_count = readstat_variable_get_missing_ranges_count(variable);
    fprintf(ctx->fp, ", \"missing\": { \"type\": \"DISCRETE\", \"values\": [");
    
    for (int i=0; i<missing_ranges_count; i++) {
        readstat_value_t lo_val = readstat_variable_get_missing_range_lo(variable, i);
        readstat_value_t hi_val = readstat_variable_get_missing_range_hi(variable, i);
        readstat_type_t lo_type = readstat_value_type(lo_val);
        
        if (i>=1) {
            fprintf(ctx->fp, ", ");
        }

        if (lo_type == READSTAT_TYPE_STRING) {
            const char *lo = readstat_string_value(lo_val);
            const char *hi = readstat_string_value(hi_val);
            if (strcmp(lo, hi) == 0) {
                fprintf(ctx->fp, "\"%s\"", lo);
            } else {
                fprintf(stderr, "%s:%d column %s unsupported string range lo '%s' hi '%s'\n", __FILE__, __LINE__, variable->name, lo, hi);
                exit(EXIT_FAILURE);
            }
        } else if (lo_type == READSTAT_TYPE_INT8) {
            char lo = readstat_int8_value(lo_val);
            char hi = readstat_int8_value(hi_val);
            if (lo == hi) {
                fprintf(ctx->fp, "%d", (int)lo);
            } else {
                fprintf(stderr, "%s:%d column %s unsupported lo %d hi %d\n", __FILE__, __LINE__, variable->name, (int)lo, (int)hi);
                exit(EXIT_FAILURE);
            }
        } else if (lo_type == READSTAT_TYPE_INT16) {
            int16_t lo = readstat_int16_value(lo_val);
            int16_t hi = readstat_int16_value(hi_val);
            if (lo == hi) {
                fprintf(ctx->fp, "%d", (int)lo);
            } else {
                fprintf(stderr, "%s:%d column %s unsupported lo %d hi %d\n", __FILE__, __LINE__, variable->name, (int)lo, (int)hi);
                exit(EXIT_FAILURE);
            }
        } else if (lo_type == READSTAT_TYPE_INT32) {
            int32_t lo = readstat_int32_value(lo_val);
            int32_t hi = readstat_int32_value(hi_val);
            if (lo == hi) {
                fprintf(ctx->fp, "%d", lo);
            } else {
                fprintf(stderr, "%s:%d column %s unsupported lo %d hi %d\n", __FILE__, __LINE__, variable->name, lo, hi);
                exit(EXIT_FAILURE);
            }
        } else if (lo_type == READSTAT_TYPE_FLOAT) {
            float lo = readstat_float_value(lo_val);
            float hi = readstat_float_value(hi_val);
            if (lo == hi) {
                fprintf(ctx->fp, "%g", lo);
            } else {
                fprintf(stderr, "%s:%d column %s unsupported lo %f hi %f\n", __FILE__, __LINE__, variable->name, lo, hi);
                exit(EXIT_FAILURE);
            }
        } else if (lo_type == READSTAT_TYPE_DOUBLE) {
            double lo = readstat_double_value(lo_val);
            double hi = readstat_double_value(hi_val);
            if (lo == hi && spss_date) {
                char buf[255];
                char *s = readstat_sav_date_string(lo, buf, sizeof(buf)-1);
                if (!s) {
                    fprintf(stderr, "Could not parse date %lf\n", lo);
                    exit(EXIT_FAILURE);
                }
                fprintf(ctx->fp, "\"%s\"", s);
            } else if (lo == hi) {
                fprintf(ctx->fp, "%g", lo);
            } else {
                fprintf(stderr, "%s:%d column %s unsupported lo %lf hi %lf\n", __FILE__, __LINE__, variable->name, lo, hi);
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "%s:%d unsupported missing type %d\n", __FILE__, __LINE__, lo_type);
            exit(EXIT_FAILURE);
        }
    }
    fprintf(ctx->fp, "]} ");
}

static void handle_missing_range(struct context *ctx, readstat_variable_t *variable) {
    const char *format = readstat_variable_get_format(variable);
    int spss_date = format && (strcmp(format, "EDATE40") == 0) && variable->type == READSTAT_TYPE_DOUBLE;
    int missing_ranges_count = readstat_variable_get_missing_ranges_count(variable);
    fprintf(ctx->fp, ", \"missing\": { \"type\": \"RANGE\", ");
    
    for (int i=0; i<missing_ranges_count; i++) {
        readstat_value_t lo_val = readstat_variable_get_missing_range_lo(variable, i);
        readstat_value_t hi_val = readstat_variable_get_missing_range_hi(variable, i);
        readstat_type_t lo_type = readstat_value_type(lo_val);
        
        if (i>=1) {
            fprintf(ctx->fp, ", ");
        }

        if (lo_type == READSTAT_TYPE_STRING) {
            const char *lo = readstat_string_value(lo_val);
            const char *hi = readstat_string_value(hi_val);
            if (strcmp(lo, hi) == 0) {
                fprintf(ctx->fp, "\"discrete-value\": \"%s\"", lo);
            } else {
                fprintf(ctx->fp, "\"low\": \"%s\", \"high\": \"%s\"", lo, hi);
            }
        } else if (lo_type == READSTAT_TYPE_INT8) {
            char lo = readstat_int8_value(lo_val);
            char hi = readstat_int8_value(hi_val);
            if (lo == hi) {
                fprintf(ctx->fp, "\"discrete-value\": %d", (int)lo);
            } else {
                fprintf(ctx->fp, "\"low\": %d, \"high\": %d", (int)lo, (int)hi);
            }
        } else if (lo_type == READSTAT_TYPE_INT16) {
            int16_t lo = readstat_int16_value(lo_val);
            int16_t hi = readstat_int16_value(hi_val);
            if (lo == hi) {
                fprintf(ctx->fp, "\"discrete-value\": %d", (int)lo);
            } else {
                fprintf(ctx->fp, "\"low\": %d, \"high\": %d", (int)lo, (int)hi);
            }
        } else if (lo_type == READSTAT_TYPE_INT32) {
            int32_t lo = readstat_int32_value(lo_val);
            int32_t hi = readstat_int32_value(hi_val);
            if (lo == hi) {
                fprintf(ctx->fp, "\"discrete-value\": %d", lo);
            } else {
                fprintf(ctx->fp, "\"low\": %d, \"high\": %d", lo, hi);
            }
        } else if (lo_type == READSTAT_TYPE_FLOAT) {
            float lo = readstat_float_value(lo_val);
            float hi = readstat_float_value(hi_val);
            if (lo == hi) {
                fprintf(ctx->fp, "\"discrete-value\": %g", lo);
            } else {
                fprintf(ctx->fp, "\"low\": %g, \"high\": %g", lo, hi);
            }
        } else if (lo_type == READSTAT_TYPE_DOUBLE) {
            double lo = readstat_double_value(lo_val);
            double hi = readstat_double_value(hi_val);
            if (spss_date) {
                char buf[255];
                char buf2[255];
                char *s = readstat_sav_date_string(lo, buf, sizeof(buf)-1);
                char *s2 = readstat_sav_date_string(hi, buf2, sizeof(buf2)-1);
                if (!s) {
                    fprintf(stderr, "Could not parse date %lf\n", lo);
                    exit(EXIT_FAILURE);
                }
                if (!s2) {
                    fprintf(stderr, "Could not parse date %lf\n", hi);
                    exit(EXIT_FAILURE);
                }
                if (lo == hi) {
                    fprintf(ctx->fp, "\"discrete-value\": \"%s\"", s);
                } else {
                    fprintf(ctx->fp, "\"low\": \"%s\", \"high\": \"%s\"", s, s2);
                }
            } else {
                if (lo == hi) {
                    fprintf(ctx->fp, "\"discrete-value\": %lf", lo);
                } else {
                    fprintf(ctx->fp, "\"low\": %lf, \"high\": %lf", lo, hi);
                }
            }
        } else {
            fprintf(stderr, "%s:%d unsupported missing type %d\n", __FILE__, __LINE__, lo_type);
            exit(EXIT_FAILURE);
        }
    }
    fprintf(ctx->fp, "} ");
}

void add_missing_values(struct context *ctx, readstat_variable_t *variable) {
    int missing_ranges_count = readstat_variable_get_missing_ranges_count(variable);
    if (missing_ranges_count == 0) {
        return;
    }
    
    int is_range = 0;
    int discrete = 0;
    int supported_type = 1;

    for (int i=0; i<missing_ranges_count; i++) {
        readstat_value_t lo_val = readstat_variable_get_missing_range_lo(variable, i);
        readstat_value_t hi_val = readstat_variable_get_missing_range_hi(variable, i);
        readstat_type_t lo_type = readstat_value_type(lo_val);
        
        // Check if types are supported (STRING, INT8, INT16, INT32, FLOAT, DOUBLE)
        if (lo_type == READSTAT_TYPE_STRING) {
            const char *lo = readstat_string_value(lo_val);
            const char *hi = readstat_string_value(hi_val);
            if (strcmp(lo, hi) != 0) {
                is_range = 1;
            } else {
                discrete = 1;
            }
        } else if (lo_type == READSTAT_TYPE_INT8) {
            char lo = readstat_int8_value(lo_val);
            char hi = readstat_int8_value(hi_val);
            if (lo != hi) {
                is_range = 1;
            } else {
                discrete = 1;
            }
        } else if (lo_type == READSTAT_TYPE_INT16) {
            int16_t lo = readstat_int16_value(lo_val);
            int16_t hi = readstat_int16_value(hi_val);
            if (lo != hi) {
                is_range = 1;
            } else {
                discrete = 1;
            }
        } else if (lo_type == READSTAT_TYPE_INT32) {
            int32_t lo = readstat_int32_value(lo_val);
            int32_t hi = readstat_int32_value(hi_val);
            if (lo != hi) {
                is_range = 1;
            } else {
                discrete = 1;
            }
        } else if (lo_type == READSTAT_TYPE_FLOAT) {
            float lo = readstat_float_value(lo_val);
            float hi = readstat_float_value(hi_val);
            if (lo != hi) {
                is_range = 1;
            } else {
                discrete = 1;
            }
        } else if (lo_type == READSTAT_TYPE_DOUBLE) {
            double lo = readstat_double_value(lo_val);
            double hi = readstat_double_value(hi_val);
            if (lo != hi) {
                is_range = 1;
            } else {
                discrete = 1;
            }
        } else {
            supported_type = 0;
        }
    }

    if (!supported_type) {
        fprintf(stderr, "%s:%d unsupported type for missing values\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    if (is_range || (is_range && discrete)) {
        handle_missing_range(ctx, variable);
    } else if (discrete) {
        handle_missing_discrete(ctx, variable);
    } else {
        fprintf(stderr, "%s:%d unexpected state\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
}
