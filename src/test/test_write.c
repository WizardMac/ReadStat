#include <stdlib.h>

#include "../readstat.h"
#include "../CKHashTable.h"

#include "test_types.h"
#include "test_buffer.h"
#include "test_readstat.h"
#include "test_dta.h"
#include "test_sas.h"

static void handle_error(const char *error_message, void *ctx) {
    printf("%s\n", error_message);
}

static ssize_t write_data(const void *bytes, size_t len, void *ctx) {
    rt_buffer_t *buffer = (rt_buffer_t *)ctx;
    while (len > buffer->size - buffer->used) {
        buffer->size *= 2;
    }
    buffer->bytes = realloc(buffer->bytes, buffer->size);
    if (buffer->bytes == NULL) {
        return -1;
    }
    memcpy(buffer->bytes + buffer->used, bytes, len);
    buffer->used += len;
    return len;
}

readstat_error_t write_file_to_buffer(rt_test_file_t *file, rt_buffer_t *buffer, long format) {
    readstat_error_t error = READSTAT_OK;

    ck_hash_table_t *label_sets = ck_hash_table_init(100);

    readstat_writer_t *writer = readstat_writer_init();
    readstat_set_data_writer(writer, &write_data);
    readstat_writer_set_file_label(writer, file->label);
    readstat_writer_set_error_handler(writer, &handle_error);
    if (file->timestamp.tm_year) {
        struct tm timestamp = file->timestamp;
        timestamp.tm_isdst = -1;
        readstat_writer_set_file_timestamp(writer, mktime(&timestamp));
    }

    if ((format & RT_FORMAT_DTA)) {
        long version = dta_file_format_version(format);
        if (version == -1) {
            error = READSTAT_ERROR_UNSUPPORTED_FILE_FORMAT_VERSION;
            goto cleanup;
        }
        readstat_writer_set_file_format_version(writer, version);
        error = readstat_begin_writing_dta(writer, buffer, file->rows);
    } else if ((format & RT_FORMAT_SAS7BDAT)) {
        if ((format & RT_FORMAT_SAS7BDAT_COMP_ROWS)) {
            readstat_writer_set_compression(writer, READSTAT_COMPRESS_ROWS);
        }
        readstat_writer_set_file_format_version(writer, sas_file_format_version(format));
        readstat_writer_set_file_format_is_64bit(writer, !!(format & RT_FORMAT_SAS7BDAT_64BIT));
        error = readstat_begin_writing_sas7bdat(writer, buffer, file->rows);
    } else if ((format & RT_FORMAT_SAS7BCAT)) {
        error = readstat_begin_writing_sas7bcat(writer, buffer);
    } else if ((format & RT_FORMAT_XPORT)) {
        readstat_writer_set_file_format_version(writer, sas_file_format_version(format));
        error = readstat_begin_writing_xport(writer, buffer, file->rows);
    } else if ((format & RT_FORMAT_SAV)) {
        if (format == RT_FORMAT_SAV_COMP_ROWS) {
            readstat_writer_set_compression(writer, READSTAT_COMPRESS_ROWS);
        }
        error = readstat_begin_writing_sav(writer, buffer, file->rows);
    } else if (format == RT_FORMAT_POR) {
        error = readstat_begin_writing_por(writer, buffer, file->rows);
    } else {
        error = READSTAT_ERROR_UNSUPPORTED_FILE_FORMAT_VERSION;
    }

    if (error != READSTAT_OK)
        goto cleanup;

    int i, j;
    int did_set_fweight = 0;
    for (j=0; j<file->notes_count; j++) {
        readstat_add_note(writer, file->notes[j]);
    }
    for (j=0; j<file->label_sets_count; j++) {
        rt_label_set_t *label_set = &file->label_sets[j];
        readstat_label_set_t *r_label_set = readstat_add_label_set(writer,
                label_set->type, label_set->name);
        for (i=0; i<label_set->value_labels_count; i++) {
            if (readstat_value_is_tagged_missing(label_set->value_labels[i].value)) {
                readstat_label_tagged_value(r_label_set,
                        readstat_value_tag(label_set->value_labels[i].value),
                        label_set->value_labels[i].label);
            } else if (label_set->type == READSTAT_TYPE_DOUBLE) {
                readstat_label_double_value(r_label_set, 
                        readstat_double_value(label_set->value_labels[i].value),
                        label_set->value_labels[i].label);
            } else if (label_set->type == READSTAT_TYPE_INT32) {
                readstat_label_int32_value(r_label_set, 
                        readstat_int32_value(label_set->value_labels[i].value),
                        label_set->value_labels[i].label);
            } else if (label_set->type == READSTAT_TYPE_STRING) {
                readstat_label_string_value(r_label_set, 
                        readstat_string_value(label_set->value_labels[i].value),
                        label_set->value_labels[i].label);
            }
        }
        ck_str_hash_insert(label_set->name, r_label_set, label_sets);
    }
    for (j=0; j<file->string_refs_count; j++) {
        readstat_add_string_ref(writer, file->string_refs[j]);
    }
    for (j=0; j<file->columns_count; j++) {
        rt_column_t *column = &file->columns[j];
        readstat_label_set_t *label_set = (readstat_label_set_t *)ck_str_hash_lookup(column->label_set, label_sets);

        size_t max_len = 0;
        if (column->type == READSTAT_TYPE_STRING) {
            max_len = 8;
            for (i=0; i<file->rows; i++) {
                const char *value = readstat_string_value(column->values[i]);
                if (value) {
                    size_t len = strlen(value);
                    if (len > max_len)
                        max_len = len;
                }
            }
            if (label_set) {
                for (i=0; i<label_set->value_labels_count; i++) {
                    if (label_set->value_labels[i].string_key_len > max_len)
                        max_len = label_set->value_labels[i].string_key_len;
                }
            }
        }

        readstat_variable_t *variable = readstat_add_variable(writer, 
                column->name, column->type, max_len);

        readstat_variable_set_alignment(variable, column->alignment);
        readstat_variable_set_measure(variable, column->measure);
        readstat_variable_set_label(variable, column->label);
        readstat_variable_set_label_set(variable, label_set);
        if (column->format[0])
            readstat_variable_set_format(variable, column->format);

        for (i=0; i<column->missing_ranges_count; i++) {
            readstat_variable_add_missing_double_range(variable,
                    readstat_double_value(column->missing_ranges[i].lo),
                    readstat_double_value(column->missing_ranges[i].hi));
        }

        if (strcmp(column->name, file->fweight) == 0) {
            error = readstat_writer_set_fweight_variable(writer, variable);
            if (error != READSTAT_OK)
                goto cleanup;

            did_set_fweight = 1;
        }
    }

    if (file->fweight[0] && !did_set_fweight) {
        error = READSTAT_ERROR_BAD_FREQUENCY_WEIGHT;
        goto cleanup;
    }

    for (i=0; i<file->rows; i++) {
        error = readstat_begin_row(writer);
        if (error != READSTAT_OK)
            goto cleanup;

        for (j=0; j<file->columns_count; j++) {
            rt_column_t *column = &file->columns[j];
            readstat_variable_t *variable = readstat_get_variable(writer, j);

            if (readstat_value_is_tagged_missing(column->values[i])) {
                error = readstat_insert_tagged_missing_value(writer, variable, 
                        readstat_value_tag(column->values[i]));
            } else if (readstat_value_is_system_missing(column->values[i])) {
                error = readstat_insert_missing_value(writer, variable);
            } else if (column->type == READSTAT_TYPE_STRING) {
                error = readstat_insert_string_value(writer, variable, 
                        readstat_string_value(column->values[i]));
            } else if (column->type == READSTAT_TYPE_STRING_REF) {
                error = readstat_insert_string_ref(writer, variable, 
                        readstat_get_string_ref(writer,
                            readstat_int32_value(column->values[i])));
            } else if (column->type == READSTAT_TYPE_DOUBLE) {
                error = readstat_insert_double_value(writer, variable, 
                        readstat_double_value(column->values[i]));
            } else if (column->type == READSTAT_TYPE_FLOAT) {
                error = readstat_insert_float_value(writer, variable, 
                        readstat_float_value(column->values[i]));
            } else if (column->type == READSTAT_TYPE_INT32) {
                error = readstat_insert_int32_value(writer, variable, 
                        readstat_int32_value(column->values[i]));
            } else if (column->type == READSTAT_TYPE_INT16) {
                error = readstat_insert_int16_value(writer, variable, 
                        readstat_int16_value(column->values[i]));
            } else if (column->type == READSTAT_TYPE_INT8) {
                error = readstat_insert_int8_value(writer, variable, 
                        readstat_int8_value(column->values[i]));
            }
            if (error != READSTAT_OK) {
                goto cleanup;
            }
        }

        error = readstat_end_row(writer);
        if (error != READSTAT_OK)
            goto cleanup;
    }
    error = readstat_end_writing(writer);
    if (error != READSTAT_OK)
        goto cleanup;

cleanup:
    ck_hash_table_free(label_sets);
    readstat_writer_free(writer);

    return error;
}

