
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <time.h>

#include "readstat_sav.h"
#include "readstat_spss_parse.h"
#include "readstat_writer.h"
#include "readstat_io.h"

#define READSTAT_PRODUCT_NAME       "ReadStat"
#define READSTAT_PRODUCT_URL        "https://github.com/WizardMac/ReadStat"

#define MAX_TEXT_SIZE               256
#define MAX_LABEL_SIZE              256

static int32_t sav_encode_format(spss_format_t *spss_format) {
    return ((spss_format->type << 16) |
            (spss_format->width << 8) |
            spss_format->decimal_places);
}

static readstat_error_t sav_encode_variable_format(int32_t *out_code, 
        readstat_variable_t *r_variable) {
    readstat_error_t retval = READSTAT_OK;
    spss_format_t spss_format;
    memset(&spss_format, 0, sizeof(spss_format_t));

    if (r_variable->type == READSTAT_TYPE_STRING) {
        spss_format.type = SPSS_FORMAT_TYPE_A;
        if (r_variable->user_width) {
            spss_format.width = r_variable->user_width;
        } else {
            spss_format.width = r_variable->width;
        }
    } else {
        spss_format.type = SPSS_FORMAT_TYPE_F;
        spss_format.width = 8;
        if (r_variable->type == READSTAT_TYPE_DOUBLE ||
                r_variable->type == READSTAT_TYPE_FLOAT) {
            spss_format.decimal_places = 2;
        }
    }

    if (r_variable->format[0]) {
        const char *fmt = r_variable->format;
        if (spss_parse_format(fmt, strlen(fmt), &spss_format) != READSTAT_OK) {
            retval = READSTAT_ERROR_BAD_FORMAT_STRING;
            goto cleanup;
        }
    } 

cleanup:
    if (retval == READSTAT_OK && out_code)
        *out_code = sav_encode_format(&spss_format);

    return retval;
}

static readstat_error_t sav_emit_header(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    time_t now = time(NULL);
    struct tm *time_s = localtime(&now);

    sav_file_header_record_t header;
    memset(&header, 0, sizeof(sav_file_header_record_t));

    memcpy(header.rec_type, "$FL2", sizeof("$FL2")-1);
    memset(header.prod_name, ' ', sizeof(header.prod_name));
    memcpy(header.prod_name,
           "@(#) SPSS DATA FILE - " READSTAT_PRODUCT_URL, 
           sizeof("@(#) SPSS DATA FILE - " READSTAT_PRODUCT_URL)-1);
    header.layout_code = 2;
    header.nominal_case_size = writer->row_len / 8;
    header.compressed = 0; /* TODO */
    if (writer->fweight_variable) {
        int32_t dictionary_index = 1 + writer->fweight_variable->offset / 8;
        header.weight_index = dictionary_index;
    } else {
        header.weight_index = 0;
    }
    header.ncases = writer->row_count;
    header.bias = 100.0;
    
    strftime(header.creation_date, sizeof(header.creation_date),
             "%d %b %y", time_s);
    strftime(header.creation_time, sizeof(header.creation_time),
             "%H:%M:%S", time_s);
    
    memset(header.file_label, ' ', sizeof(header.file_label));

    size_t file_label_len = strlen(writer->file_label);
    if (file_label_len > sizeof(header.file_label))
        file_label_len = sizeof(header.file_label);

    if (writer->file_label[0])
        memcpy(header.file_label, writer->file_label, file_label_len);
    
    retval = readstat_write_bytes(writer, &header, sizeof(header));
    return retval;
}

static readstat_error_t sav_emit_variable_label(readstat_writer_t *writer, readstat_variable_t *r_variable) {
    readstat_error_t retval = READSTAT_OK;
    const char *title_data = r_variable->label;
    size_t title_data_len = strlen(title_data);
    if (title_data_len > 0) {
        char padded_label[MAX_LABEL_SIZE];
        int32_t label_len = title_data_len;
        if (label_len > sizeof(padded_label))
            label_len = sizeof(padded_label);

        retval = readstat_write_bytes(writer, &label_len, sizeof(label_len));
        if (retval != READSTAT_OK)
            goto cleanup;

        strncpy(padded_label, title_data, (label_len + 3) / 4 * 4);

        retval = readstat_write_bytes(writer, padded_label, (label_len + 3) / 4 * 4);
        if (retval != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    return retval;
}

static int sav_n_missing_values(readstat_variable_t *r_variable) {
    int n_missing_ranges = readstat_variable_get_missing_ranges_count(r_variable);
    int n_missing_values = n_missing_ranges;
    int has_missing_range = 0;
    int j;
    for (j=0; j<n_missing_ranges; j++) {
        readstat_value_t lo = readstat_variable_get_missing_range_lo(r_variable, j);
        readstat_value_t hi = readstat_variable_get_missing_range_hi(r_variable, j);
        if (spss_64bit_value(lo) != spss_64bit_value(hi)) {
            n_missing_values++;
            has_missing_range = 1;
        }
    }
    if (n_missing_values > 3)
        n_missing_values = 3;

    return has_missing_range ? -n_missing_values : n_missing_values;
}

static readstat_error_t sav_emit_variable_missing_values(readstat_writer_t *writer, readstat_variable_t *r_variable) {
    readstat_error_t retval = READSTAT_OK;
    int n_missing_values = 0;
    int n_missing_ranges = readstat_variable_get_missing_ranges_count(r_variable);
    /* ranges */
    int j;

    for (j=0; j<n_missing_ranges; j++) {
        readstat_value_t lo = readstat_variable_get_missing_range_lo(r_variable, j);
        readstat_value_t hi = readstat_variable_get_missing_range_hi(r_variable, j);
        if (spss_64bit_value(lo) != spss_64bit_value(hi)) {
            uint64_t lo_val = spss_64bit_value(lo);
            retval = readstat_write_bytes(writer, &lo_val, sizeof(uint64_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            uint64_t hi_val = spss_64bit_value(hi);
            retval = readstat_write_bytes(writer, &hi_val, sizeof(uint64_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            n_missing_values += 2;

            break;
        }
    }
    /* values */
    for (j=0; j<n_missing_ranges; j++) {
        readstat_value_t lo = readstat_variable_get_missing_range_lo(r_variable, j);
        readstat_value_t hi = readstat_variable_get_missing_range_hi(r_variable, j);
        if (spss_64bit_value(lo) == spss_64bit_value(hi)) {
            uint64_t d_val = spss_64bit_value(lo);
            retval = readstat_write_bytes(writer, &d_val, sizeof(uint64_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            if (++n_missing_values == 3)
                break;
        }
    }
cleanup:
    return retval;
}

static readstat_error_t sav_emit_variable_records(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i, j;
    int32_t rec_type = 0;
    
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);
        char name_data[9];
        snprintf(name_data, sizeof(name_data), "VAR%d", i);
        size_t name_data_len = strlen(name_data);

        rec_type = SAV_RECORD_TYPE_VARIABLE;
        retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
        if (retval != READSTAT_OK)
            goto cleanup;
        
        sav_variable_record_t variable;
        memset(&variable, 0, sizeof(sav_variable_record_t));

        variable.type = (r_variable->type == READSTAT_TYPE_STRING) ? r_variable->width : 0;
        variable.has_var_label = (r_variable->label[0] != '\0');
        variable.n_missing_values = sav_n_missing_values(r_variable); 

        retval = sav_encode_variable_format(&variable.print, r_variable);
        if (retval != READSTAT_OK)
            goto cleanup;

        variable.write = variable.print;

        memset(variable.name, ' ', sizeof(variable.name));
        if (name_data_len > 0 && name_data_len <= sizeof(variable.name))
            memcpy(variable.name, name_data, name_data_len);
        
        retval = readstat_write_bytes(writer, &variable, sizeof(variable));
        if (retval != READSTAT_OK)
            goto cleanup;
        
        retval = sav_emit_variable_label(writer, r_variable);
        if (retval != READSTAT_OK)
            goto cleanup;

        retval = sav_emit_variable_missing_values(writer, r_variable);
        if (retval != READSTAT_OK)
            goto cleanup;
        
        int extra_fields = r_variable->width / 8 - 1;
        for (j=0; j<extra_fields; j++) {
            retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
            if (retval != READSTAT_OK)
                goto cleanup;

            memset(&variable, '\0', sizeof(variable));
            variable.type = -1;
            retval = readstat_write_bytes(writer, &variable, sizeof(variable));
            if (retval != READSTAT_OK)
                goto cleanup;
        }
    }

cleanup:
    return retval;
}

static readstat_error_t sav_emit_value_label_records(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i, j;
    for (i=0; i<writer->label_sets_count; i++) {
        readstat_label_set_t *r_label_set = readstat_get_label_set(writer, i);
        int32_t label_count = r_label_set->value_labels_count;
        int32_t rec_type = 0;
        if (label_count) {
            rec_type = SAV_RECORD_TYPE_VALUE_LABEL;

            retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, &label_count, sizeof(label_count));
            if (retval != READSTAT_OK)
                goto cleanup;
            
            readstat_types_t user_type = r_label_set->type;
            for (j=0; j<label_count; j++) {
                readstat_value_label_t *r_value_label = readstat_get_value_label(r_label_set, j);
                char value[8];
                if (user_type == READSTAT_TYPE_STRING) {
                    const char *txt_value = r_value_label->string_key;
                    size_t txt_len = strlen(txt_value);
                    if (txt_len > 8)
                        txt_len = 8;

                    memset(value, ' ', sizeof(value));
                    memcpy(value, txt_value, txt_len);
                } else if (user_type == READSTAT_TYPE_DOUBLE) {
                    double num_val = r_value_label->double_key;
                    memcpy(value, &num_val, sizeof(double));
                } else if (user_type == READSTAT_TYPE_INT32) {
                    double num_val = r_value_label->int32_key;
                    memcpy(value, &num_val, sizeof(double));
                }
                retval = readstat_write_bytes(writer, value, sizeof(value));
                
                const char *label_data = r_value_label->label;
                size_t label_data_len = strlen(label_data);
                if (label_data_len > 255)
                    label_data_len = 255;
                
                char label_len = label_data_len;
                retval = readstat_write_bytes(writer, &label_len, sizeof(label_len));
                if (retval != READSTAT_OK)
                    goto cleanup;

                char label[255];
                memset(label, ' ', sizeof(label));
                memcpy(label, label_data, label_data_len);
                retval = readstat_write_bytes(writer, label, (label_data_len + 8) / 8 * 8 - 1);
                if (retval != READSTAT_OK)
                    goto cleanup;
            }
            
            rec_type = SAV_RECORD_TYPE_VALUE_LABEL_VARIABLES;
            int32_t var_count = r_label_set->variables_count;
            
            retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, &var_count, sizeof(var_count));
            if (retval != READSTAT_OK)
                goto cleanup;

            for (j=0; j<var_count; j++) {
                readstat_variable_t *r_variable = readstat_get_label_set_variable(r_label_set, j);
                int32_t dictionary_index = 1 + r_variable->offset / 8;
                retval = readstat_write_bytes(writer, &dictionary_index, sizeof(dictionary_index));
                if (retval != READSTAT_OK)
                    goto cleanup;
            }
        }
    }

cleanup:
    return retval;
}

static readstat_error_t sav_emit_integer_info_record(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;

    sav_info_record_t info_header;
    memset(&info_header, 0, sizeof(sav_info_record_t));

    info_header.rec_type = SAV_RECORD_TYPE_HAS_DATA;
    info_header.subtype = SAV_RECORD_SUBTYPE_INTEGER_INFO;
    info_header.size = 4;
    info_header.count = 8;

    retval = readstat_write_bytes(writer, &info_header, sizeof(info_header));
    if (retval != READSTAT_OK)
        goto cleanup;
    
    sav_machine_integer_info_record_t machine_info;
    memset(&machine_info, 0, sizeof(sav_machine_integer_info_record_t));

    machine_info.version_major = 1;
    machine_info.version_minor = 0;
    machine_info.version_revision = 0;
    machine_info.machine_code = -1;
    machine_info.floating_point_rep = SAV_FLOATING_POINT_REP_IEEE;
    machine_info.compression_code = 1;
    machine_info.endianness = machine_is_little_endian() ? SAV_ENDIANNESS_LITTLE : SAV_ENDIANNESS_BIG;
    machine_info.character_code = SAV_CHARSET_UTF8;

    retval = readstat_write_bytes(writer, &machine_info, sizeof(machine_info));

cleanup:
    return retval;
}

static readstat_error_t sav_emit_floating_point_info_record(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;

    sav_info_record_t info_header;
    memset(&info_header, 0, sizeof(sav_info_record_t));

    info_header.rec_type = SAV_RECORD_TYPE_HAS_DATA;
    info_header.subtype = SAV_RECORD_SUBTYPE_FP_INFO;
    info_header.size = 8;
    info_header.count = 3;

    retval = readstat_write_bytes(writer, &info_header, sizeof(info_header));
    if (retval != READSTAT_OK)
        goto cleanup;
    
    sav_machine_floating_point_info_record_t fp_info;
    memset(&fp_info, 0, sizeof(sav_machine_floating_point_info_record_t));

    fp_info.sysmis = NAN;
    fp_info.lowest = DBL_MAX;
    fp_info.highest = -DBL_MAX;

    retval = readstat_write_bytes(writer, &fp_info, sizeof(fp_info));
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t sav_emit_long_var_name_record(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    sav_info_record_t info_header;
    memset(&info_header, 0, sizeof(sav_info_record_t));

    info_header.rec_type = SAV_RECORD_TYPE_HAS_DATA;
    info_header.subtype = SAV_RECORD_SUBTYPE_LONG_VAR_NAME;
    info_header.size = 1;
    info_header.count = 0;
    
    for (i=0; i<writer->variables_count; i++) {
        char name_data[9];
        snprintf(name_data, sizeof(name_data), "VAR%d", i);
        size_t name_data_len = strlen(name_data);
        
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);
        const char *title_data = r_variable->name;
        size_t title_data_len = strlen(title_data);
        if (title_data_len > 0 && name_data_len > 0) {
            if (title_data_len > 64)
                title_data_len = 64;
            
            info_header.count += name_data_len;
            info_header.count += sizeof("=")-1;
            info_header.count += title_data_len;
            info_header.count += sizeof("\x09")-1;
        }
    }
    
    if (info_header.count > 0) {
        info_header.count--; /* no trailing 0x09 */
        
        retval = readstat_write_bytes(writer, &info_header, sizeof(info_header));
        if (retval != READSTAT_OK)
            goto cleanup;
        
        int is_first = 1;
        
        for (i=0; i<writer->variables_count; i++) {
            char name_data[9];
            snprintf(name_data, sizeof(name_data), "VAR%d", i);
            size_t name_data_len = strlen(name_data);

            readstat_variable_t *r_variable = readstat_get_variable(writer, i);
            const char *title_data = r_variable->name;
            size_t title_data_len = strlen(title_data);
            
            char kv_separator = '=';
            char tuple_separator = 0x09;
            
            if (title_data_len > 0) {
                if (title_data_len > 64)
                    title_data_len = 64;

                if (!is_first) {
                    retval = readstat_write_bytes(writer, &tuple_separator, sizeof(tuple_separator));
                    if (retval != READSTAT_OK)
                        goto cleanup;
                }
                
                retval = readstat_write_bytes(writer, name_data, name_data_len);
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_bytes(writer, &kv_separator, sizeof(kv_separator));
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_bytes(writer, title_data, title_data_len);
                if (retval != READSTAT_OK)
                    goto cleanup;
                
                is_first = 0;
            }
        }
    }

cleanup:
    return retval;
}

static readstat_error_t sav_emit_termination_record(readstat_writer_t *writer) {
    sav_dictionary_termination_record_t termination_record;
    memset(&termination_record, 0, sizeof(sav_dictionary_termination_record_t));

    termination_record.rec_type = SAV_RECORD_TYPE_DICT_TERMINATION;
    
    return readstat_write_bytes(writer, &termination_record, sizeof(termination_record));
}

static readstat_error_t sav_write_char(void *row, const readstat_variable_t *var, char value) {
    if (var->type != READSTAT_TYPE_CHAR) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_int16(void *row, const readstat_variable_t *var, int16_t value) {
    if (var->type != READSTAT_TYPE_INT16) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_int32(void *row, const readstat_variable_t *var, int32_t value) {
    if (var->type != READSTAT_TYPE_INT32) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_float(void *row, const readstat_variable_t *var, float value) {
    if (var->type != READSTAT_TYPE_FLOAT) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_double(void *row, const readstat_variable_t *var, double value) {
    if (var->type != READSTAT_TYPE_DOUBLE) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_string(void *row, const readstat_variable_t *var, const char *value) {
    if (var->type != READSTAT_TYPE_STRING) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    memset(row, ' ', var->width);
    if (value != NULL && value[0] != '\0') {
        memcpy(row, value, strlen(value));
    }
    return READSTAT_OK;
}

static readstat_error_t sav_write_missing(void *row, const readstat_variable_t *var) {
    if (var->type == READSTAT_TYPE_STRING) {
        memset(row, ' ', var->width);
    } else {
        uint64_t missing_val = SAV_MISSING_DOUBLE;
        memcpy(row, &missing_val, sizeof(uint64_t));
    }
    return READSTAT_OK;
}

static size_t sav_variable_width(readstat_types_t type, size_t user_width) {
    if (type == READSTAT_TYPE_STRING) {
        if (user_width > 255 || user_width == 0)
            user_width = 255;
        return (user_width + 7) / 8 * 8;
    }
    return 8;
}

static readstat_error_t sav_begin_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    readstat_error_t retval = READSTAT_OK;

    retval = sav_emit_header(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_variable_records(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_value_label_records(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_integer_info_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_floating_point_info_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_long_var_name_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_termination_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

readstat_error_t readstat_begin_writing_sav(readstat_writer_t *writer, void *user_ctx, long row_count) {
    writer->row_count = row_count;
    writer->user_ctx = user_ctx;

    writer->callbacks.variable_width = &sav_variable_width;
    writer->callbacks.write_char = &sav_write_char;
    writer->callbacks.write_int16 = &sav_write_int16;
    writer->callbacks.write_int32 = &sav_write_int32;
    writer->callbacks.write_float = &sav_write_float;
    writer->callbacks.write_double = &sav_write_double;
    writer->callbacks.write_string = &sav_write_string;
    writer->callbacks.write_missing = &sav_write_missing;
    writer->callbacks.begin_data = &sav_begin_data;

    return READSTAT_OK;
}
