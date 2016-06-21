#include <stdlib.h>

#include "../readstat.h"

#include "test_types.h"
#include "test_buffer.h"
#include "test_readstat.h"

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

    readstat_writer_t *writer = readstat_writer_init();
    readstat_set_data_writer(writer, &write_data);
    readstat_writer_set_file_label(writer, "ReadStat Test File");

    if (format == RT_FORMAT_DTA_104) {
        error = readstat_begin_writing_dta(writer, 104, buffer, file->rows);
    } else if (format == RT_FORMAT_DTA_105) {
        error = readstat_begin_writing_dta(writer, 105, buffer, file->rows);
    } else if (format == RT_FORMAT_DTA_108) {
        error = readstat_begin_writing_dta(writer, 108, buffer, file->rows);
    } else if (format == RT_FORMAT_DTA_110) {
        error = readstat_begin_writing_dta(writer, 110, buffer, file->rows);
    } else if (format == RT_FORMAT_DTA_111) {
        error = readstat_begin_writing_dta(writer, 111, buffer, file->rows);
    } else if (format == RT_FORMAT_DTA_114) {
        error = readstat_begin_writing_dta(writer, 114, buffer, file->rows);
    } else if (format == RT_FORMAT_DTA_117) {
        error = readstat_begin_writing_dta(writer, 117, buffer, file->rows);
    } else if (format == RT_FORMAT_DTA_118) {
        error = readstat_begin_writing_dta(writer, 118, buffer, file->rows);
    } else if (format == RT_FORMAT_SAV) {
        error = readstat_begin_writing_sav(writer, buffer, file->rows);
    } else {
        error = READSTAT_ERROR_UNSUPPORTED_FILE_FORMAT_VERSION;
    }

    if (error != READSTAT_OK)
        goto cleanup;

    int i, j;
    for (j=0; j<file->columns_count; j++) {
        rt_column_t *column = &file->columns[j];

        readstat_variable_t *variable = readstat_add_variable(writer, 
                column->name, column->type, RT_MAX_STRING);

        readstat_variable_set_alignment(variable, column->alignment);
        readstat_variable_set_measure(variable, column->measure);
        readstat_variable_set_label(variable, column->label);
    }

    for (i=0; i<file->rows; i++) {
        error = readstat_begin_row(writer);
        if (error != READSTAT_OK)
            goto cleanup;

        for (j=0; j<file->columns_count; j++) {
            rt_column_t *column = &file->columns[j];
            readstat_variable_t *variable = readstat_get_variable(writer, j);

            if (readstat_value_tag(column->values[i])) {
                error = readstat_insert_tagged_missing_value(writer, variable, 
                        readstat_value_tag(column->values[i]));
            } else if (column->type == READSTAT_TYPE_STRING ||
                    column->type == READSTAT_TYPE_LONG_STRING) {
                error = readstat_insert_string_value(writer, variable, 
                        readstat_string_value(column->values[i]));
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
            } else if (column->type == READSTAT_TYPE_CHAR) {
                error = readstat_insert_char_value(writer, variable, 
                        readstat_char_value(column->values[i]));
            }
            if (error != READSTAT_OK)
                goto cleanup;
        }

        error = readstat_end_row(writer);
        if (error != READSTAT_OK)
            goto cleanup;
    }
    error = readstat_end_writing(writer);
    if (error != READSTAT_OK)
        goto cleanup;

cleanup:
    readstat_writer_free(writer);

    return error;
}

