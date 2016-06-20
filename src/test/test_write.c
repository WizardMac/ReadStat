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

readstat_error_t write_file_to_buffer(rt_file_t *file, rt_buffer_t *buffer, long format) {
    readstat_error_t error = READSTAT_OK;

    readstat_writer_t *writer = readstat_writer_init();
    readstat_set_data_writer(writer, &write_data);
    readstat_writer_set_file_label(writer, "ReadStat Test File");

    if (format == RT_FORMAT_DTA) {
        error = readstat_begin_writing_dta(writer, buffer, RT_MAX_ROWS);
    } else if (format == RT_FORMAT_SAV) {
        error = readstat_begin_writing_sav(writer, buffer, RT_MAX_ROWS);
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

    for (i=0; i<RT_MAX_ROWS; i++) {
        error = readstat_begin_row(writer);
        if (error != READSTAT_OK)
            goto cleanup;

        for (j=0; j<file->columns_count; j++) {
            rt_column_t *column = &file->columns[j];
            readstat_variable_t *variable = readstat_get_variable(writer, j);

            if (column->type == READSTAT_TYPE_STRING ||
                    column->type == READSTAT_TYPE_LONG_STRING) {
                error = readstat_insert_string_value(writer, variable, column->string_values[i]);
            } else if (column->type == READSTAT_TYPE_DOUBLE) {
                error = readstat_insert_double_value(writer, variable, column->double_values[i]);
            } else if (column->type == READSTAT_TYPE_FLOAT) {
                error = readstat_insert_float_value(writer, variable, column->float_values[i]);
            } else if (column->type == READSTAT_TYPE_INT32) {
                error = readstat_insert_int32_value(writer, variable, column->i32_values[i]);
            } else if (column->type == READSTAT_TYPE_INT16) {
                error = readstat_insert_int16_value(writer, variable, column->i16_values[i]);
            } else if (column->type == READSTAT_TYPE_CHAR) {
                error = readstat_insert_char_value(writer, variable, column->i8_values[i]);
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

