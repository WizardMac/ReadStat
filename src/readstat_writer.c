
#include <stdlib.h>
#include "readstat.h"

readstat_writer_t *readstat_writer_init() {
    readstat_writer_t *writer = calloc(1, sizeof(readstat_writer_t));
    return writer;
}

void readstat_writer_free(readstat_writer_t *writer) {
    if (writer)
        free(writer);
}

readstat_error_t readstat_set_data_writer(readstat_writer_t *writer, readstat_data_writer data_writer) {
    writer->data_writer = data_writer;
    return READSTAT_OK;
}

readstat_error_t readstat_set_variable_shortname_provider(readstat_writer_t *writer, readstat_variable_name_provider variable_name_provider) {
    writer->variable_shortname_provider = variable_name_provider;
    return READSTAT_OK;
}

readstat_error_t readstat_set_variable_longname_provider(readstat_writer_t *writer, readstat_variable_name_provider variable_name_provider) {
    writer->variable_longname_provider = variable_name_provider;
    return READSTAT_OK;
}

readstat_error_t readstat_set_variable_type_provider(readstat_writer_t *writer, readstat_variable_type_provider variable_type_provider) {
    writer->variable_type_provider = variable_type_provider;
    return READSTAT_OK;
}

readstat_error_t readstat_set_variable_format_provider(readstat_writer_t *writer, readstat_variable_format_provider variable_format_provider) {
    writer->variable_format_provider = variable_format_provider;
    return READSTAT_OK;
}

readstat_error_t readstat_set_variable_width_provider(readstat_writer_t *writer, readstat_variable_width_provider variable_width_provider) {
    writer->variable_width_provider = variable_width_provider;
    return READSTAT_OK;
}

readstat_error_t readstat_set_value_provider(readstat_writer_t *writer, readstat_value_provider value_provider) {
    writer->value_provider = value_provider;
    return READSTAT_OK;
}

readstat_error_t readstat_set_vlabel_count_provider(readstat_writer_t *writer, readstat_vlabel_count_provider vlabel_count_provider) {
    writer->vlabel_count_provider = vlabel_count_provider;
    return READSTAT_OK;
}
readstat_error_t readstat_set_vlabel_double_value_provider(readstat_writer_t *writer, readstat_vlabel_double_provider vlabel_value_provider) {
    writer->vlabel_double_value_provider = vlabel_value_provider;
    return READSTAT_OK;
}

readstat_error_t readstat_set_vlabel_string_value_provider(readstat_writer_t *writer, readstat_vlabel_string_provider vlabel_value_provider) {
    writer->vlabel_string_value_provider = vlabel_value_provider;
    return READSTAT_OK;
}

readstat_error_t readstat_set_vlabel_label_provider(readstat_writer_t *writer, readstat_vlabel_string_provider vlabel_label_provider) {
    writer->vlabel_label_provider = vlabel_label_provider;
    return READSTAT_OK;
}

readstat_error_t readstat_set_row_count(readstat_writer_t *writer, int row_count) {
    writer->obs_count = row_count;
    return READSTAT_OK;
}

readstat_error_t readstat_set_var_count(readstat_writer_t *writer, int var_count) {
    writer->var_count = var_count;
    return READSTAT_OK;
}

readstat_error_t readstat_set_file_label(readstat_writer_t *writer, const char *file_label) {
    snprintf(writer->file_label, sizeof(writer->file_label), "%s", file_label);
    return READSTAT_OK;
}
