
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../readstat.h"
#include "../readstat_writer.h"
#include "readstat_sas.h"
#include "ieee.h"

#define XPORT_DEFAULT_VERISON 8

#pragma pack(push, 1)
typedef struct xport_namestr_s {
    uint16_t    ntype;
    uint16_t    nhfun;
    uint16_t    nlng;
    uint16_t    nvar0;
    char        nname[8];
    char        nlabel[40];
    char        nform[8];
    uint16_t    nfl;
    uint16_t    nfd;
    uint16_t    nfj;
    char        nfill[2];
    char        niform[8];
    uint16_t    nifl;
    uint16_t    nifd;
    uint32_t    npos;
    char        longname[32];
    uint16_t    labeln;
    char        rest[18];
} xport_namestr_t;
#pragma pack(pop)

static readstat_error_t xport_write_bytes(readstat_writer_t *writer, const void *bytes, size_t len) {
    return readstat_write_bytes_as_lines(writer, bytes, len, 80, "\n");
}

static readstat_error_t xport_finish_record(readstat_writer_t *writer) {
    return readstat_write_line_padding(writer, ' ', 80, "\n");
}

static readstat_error_t xport_write_record(readstat_writer_t *writer, const char *record) {
    size_t len = strlen(record);
    readstat_error_t retval = READSTAT_OK;
    
    retval = xport_write_bytes(writer, record, len);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_finish_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t xport_write_header_record(readstat_writer_t *writer, char *name,
        unsigned int num1, unsigned int num2, unsigned int num3,
        unsigned int num4, unsigned int num5, unsigned int num6) {
    char record[81];
    snprintf(record, sizeof(record),
            "HEADER RECORD*******%-8sHEADER RECORD!!!!!!!%010d%010d%010d",
            name, num1, num2, num3);
    return xport_write_record(writer, record);
}

static size_t xport_variable_width(readstat_type_t type, size_t user_width) {
    if (type == READSTAT_TYPE_STRING) {
        return user_width;
    }
    return 8;
}

static readstat_error_t xport_write_variables(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    long offset = 0;
    int num_long_labels = 0;
    int has_long_format = 0;
    for (i=0; i<writer->variables_count; i++) {
        int needs_long_record = 0;
        readstat_variable_t *variable = readstat_get_variable(writer, i);
        size_t width = xport_variable_width(variable->type, variable->user_width);
        xport_namestr_t namestr = { 
            .nvar0 = i,
            .nlng = width,
            .npos = offset
        };
        if (readstat_variable_get_type_class(variable) == READSTAT_TYPE_CLASS_STRING) {
            namestr.ntype = SAS_COLUMN_TYPE_CHR;
        } else {
            namestr.ntype = SAS_COLUMN_TYPE_NUM;
        }
        strncpy(namestr.nname, variable->name, sizeof(namestr.nname));
        strncpy(namestr.nlabel, variable->label, sizeof(namestr.nlabel));

        if (variable->format[0]) {
            int decimals = 2;
            int width = 8;
            char name[24];

            sscanf(variable->format, "%s%d.%d", name, &width, &decimals);

            strncpy(namestr.nform, name, sizeof(namestr.nform));
            namestr.nfl = width;
            namestr.nfd = decimals;

            strncpy(namestr.niform, name, sizeof(namestr.niform));
            namestr.nifl = width;
            namestr.nifd = decimals;

            if (strlen(name) > 8) {
                has_long_format = 1;
                needs_long_record = 1;
            }
        }

        if (writer->version == 8) {
            strncpy(namestr.longname, variable->name, sizeof(namestr.longname));

            size_t label_len = strlen(variable->label);
            if (label_len > 40) {
                needs_long_record = 1;
            }
            namestr.labeln = label_len;
        }

        if (needs_long_record) {
            num_long_labels++;
        }

        offset += width;

        retval = xport_write_bytes(writer, &namestr, sizeof(xport_namestr_t));
        if (retval != READSTAT_OK)
            goto cleanup;
    }

    if (writer->version == 8 && num_long_labels) {
        if (has_long_format) {
            retval = xport_write_header_record(writer, "LABELV9", num_long_labels, 0, 0, 0, 0, 0);
            if (retval != READSTAT_OK)
                goto cleanup;
        } else {
            retval = xport_write_header_record(writer, "LABELV8", num_long_labels, 0, 0, 0, 0, 0);
            if (retval != READSTAT_OK)
                goto cleanup;
        }

        for (i=0; i<writer->variables_count; i++) {
            readstat_variable_t *variable = readstat_get_variable(writer, i);
            size_t label_len = strlen(variable->label);
            size_t name_len = strlen(variable->name);
            int has_long_label = 0;
            int has_long_format = 0;
            int format_len = 0;
            char labeldef[11];
            char format_name[24];
            memset(format_name, 0, sizeof(format_name));

            has_long_label = (label_len > 40);

            if (variable->format[0]) {
                int decimals = 2;
                int width = 8;

                sscanf(variable->format, "%s%d.%d", format_name, &width, &decimals);
            }

            if (has_long_format) {
                snprintf(labeldef, sizeof(labeldef), "%02d%02d%02d%02d%02d", 
                        i, (int)name_len, (int)format_len, (int)format_len, (int)label_len);

                retval = readstat_write_string(writer, labeldef);
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_string(writer, variable->name);
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_string(writer, format_name);
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_string(writer, format_name);
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_string(writer, variable->label);
                if (retval != READSTAT_OK)
                    goto cleanup;

            } else if (has_long_label) {
                snprintf(labeldef, sizeof(labeldef), "%02d%02d%02d", 
                        i, (int)name_len, (int)label_len);

                retval = readstat_write_string(writer, labeldef);
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_string(writer, variable->name);
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_string(writer, variable->label);
                if (retval != READSTAT_OK)
                    goto cleanup;
            }
        }
    }

    retval = xport_finish_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:

    return retval;
}

static readstat_error_t xport_begin_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    struct tm *ts = localtime(&writer->timestamp);
    readstat_error_t retval = READSTAT_OK;

    retval = sas_validate_column_names(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    if (writer->version == 5) {
        retval = xport_write_header_record(writer, "LIBRARY", 0, 0, 0, 0, 0, 0);
    } else {
        retval = xport_write_header_record(writer, "LIBV8",   0, 0, 0, 0, 0, 0);
    }
    if (retval != READSTAT_OK)
        goto cleanup;

    char month[12][4] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

    char timestamp[17];
    snprintf(timestamp, sizeof(timestamp),
            "%02d"       "%3s"              "%02d"             ":%02d:%02d:%02d",
            ts->tm_mday, month[ts->tm_mon], ts->tm_year % 100, ts->tm_hour, ts->tm_min, ts->tm_sec);

    char real_record[81];
    snprintf(real_record, sizeof(real_record),
            "%-8s" "%-8s" "%-8s"    "%-8s"  "%-8s"    "%-24s" "%16s",
            "SAS", "SAS", "SASLIB", "6.06", "bsd4.2", "",     timestamp);

    retval = xport_write_record(writer, real_record);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_write_record(writer, timestamp);
    if (retval != READSTAT_OK)
        goto cleanup;

    if (writer->version == 5) {
        retval = xport_write_header_record(writer, "MEMBER", 0, 0, 0, 160, 0, 140);
    } else {
        retval = xport_write_header_record(writer, "MEMBV8", 0, 0, 0, 160, 0, 140);
    }
    if (retval != READSTAT_OK)
        goto cleanup;

    if (writer->version == 5) {
        retval = xport_write_header_record(writer, "DSCRPTR", 0, 0, 0, 0, 0, 0);
    } else {
        retval = xport_write_header_record(writer, "DSCPTV8", 0, 0, 0, 0, 0, 0);
    }
    if (retval != READSTAT_OK)
        goto cleanup;

    char member_header[81];
    snprintf(member_header, sizeof(member_header),
            "%-8s" "%-8s"     "%-8s"    "%-8s"  "%-8s"    "%-24s" "%16s",
            "SAS", "DATASET", "SASLIB", "6.06", "bsd4.2", "", timestamp);

    retval = xport_write_record(writer, member_header);
    if (retval != READSTAT_OK)
        goto cleanup;

    snprintf(member_header, sizeof(member_header),
            "%16s"     "%16s"  "%-40s"              "%-8s",
            timestamp, "",     writer->file_label,  "" /* dstype? */);

    retval = xport_write_record(writer, member_header);
    if (retval != READSTAT_OK)
        goto cleanup;

    if (writer->version == 5) {
        retval = xport_write_header_record(writer, "NAMESTR", 0, writer->variables_count, 0, 0, 0, 0);
    } else {
        retval = xport_write_header_record(writer, "NAMSTV8", 0, writer->variables_count, 0, 0, 0, 0);
    }
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_write_variables(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    if (writer->version == 5) {
        retval = xport_write_header_record(writer, "OBS", 0, 0, 0, 0, 0, 0);
    } else {
        retval = xport_write_header_record(writer, "OBSV8", 0, 0, 0, 0, 0, 0);
    }
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t xport_end_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    readstat_error_t retval = READSTAT_OK;

    retval = xport_finish_record(writer);

    return retval;
}

static readstat_error_t xport_write_row(void *writer_ctx, void *row, size_t row_len) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    return xport_write_bytes(writer, row, row_len);
}

static readstat_error_t xport_write_double(void *row, const readstat_variable_t *var, double value) {
    int rc = cnxptiee(&value, CN_TYPE_NATIVE, row, CN_TYPE_XPORT);

    if (rc == 0)
        return READSTAT_OK;

    return READSTAT_ERROR_CONVERT;
}

static readstat_error_t xport_write_float(void *row, const readstat_variable_t *var, float value) {
    return xport_write_double(row, var, value);
}

static readstat_error_t xport_write_int32(void *row, const readstat_variable_t *var, int32_t value) {
    return xport_write_double(row, var, value);
}

static readstat_error_t xport_write_int16(void *row, const readstat_variable_t *var, int16_t value) {
    return xport_write_double(row, var, value);
}

static readstat_error_t xport_write_int8(void *row, const readstat_variable_t *var, int8_t value) {
    return xport_write_double(row, var, value);
}

static readstat_error_t xport_write_string(void *row, const readstat_variable_t *var, const char *string) {
    memset(row, ' ', var->storage_width);
    if (string != NULL && string[0]) {
        size_t value_len = strlen(string);
        if (value_len > var->storage_width)
            return READSTAT_ERROR_STRING_VALUE_IS_TOO_LONG;

        memcpy(row, string, value_len);
    }
    return READSTAT_OK;
}

static readstat_error_t xport_write_missing_numeric(void *row, const readstat_variable_t *var) {
    char *row_bytes = (char *)row;
    row_bytes[0] = 0x2e;
    return READSTAT_OK;
}   
    
static readstat_error_t xport_write_missing_string(void *row, const readstat_variable_t *var) {
    return xport_write_string(row, var, NULL);
}

static readstat_error_t xport_write_missing_tagged(void *row, const readstat_variable_t *var, char tag) {
    char *row_bytes = (char *)row;
    if (tag == '_' || (tag >= 'A' && tag <= 'Z')) {
        row_bytes[0] = tag;
        return READSTAT_OK;
    }
    return READSTAT_ERROR_TAGGED_VALUE_IS_OUT_OF_RANGE;
}

readstat_error_t readstat_begin_writing_xport(readstat_writer_t *writer, void *user_ctx, long row_count) {

    if (writer->version == 0)
        writer->version = XPORT_DEFAULT_VERISON;

    if (writer->version != 5 && writer->version != 8)
        return READSTAT_ERROR_UNSUPPORTED_FILE_FORMAT_VERSION;

    writer->callbacks.write_int8 = &xport_write_int8;
    writer->callbacks.write_int16 = &xport_write_int16;
    writer->callbacks.write_int32 = &xport_write_int32;
    writer->callbacks.write_float = &xport_write_float;
    writer->callbacks.write_double = &xport_write_double;

    writer->callbacks.write_string = &xport_write_string;
    writer->callbacks.write_missing_string = &xport_write_missing_string;
    writer->callbacks.write_missing_number = &xport_write_missing_numeric;
    writer->callbacks.write_missing_tagged = &xport_write_missing_tagged;

    writer->callbacks.variable_width = &xport_variable_width;

    writer->callbacks.begin_data = &xport_begin_data;
    writer->callbacks.end_data = &xport_end_data;

    writer->callbacks.write_row = &xport_write_row;

    return readstat_begin_writing_file(writer, user_ctx, row_count);
}
