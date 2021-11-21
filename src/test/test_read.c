#include <stdlib.h>

#include "../readstat.h"

#include "test_buffer.h"
#include "test_types.h"
#include "test_error.h"
#include "test_buffer_io.h"
#include "test_readstat.h"
#include "test_read.h"
#include "test_dta.h"
#include "test_sas.h"
#include "test_sav.h"

char *file_extension(long format) {
    if (format == RT_FORMAT_DTA_104)
        return "dta104";
    if (format == RT_FORMAT_DTA_105)
        return "dta105";
    if (format == RT_FORMAT_DTA_108)
        return "dta108";
    if (format == RT_FORMAT_DTA_110)
        return "dta110";
    if (format == RT_FORMAT_DTA_111)
        return "dta111";
    if (format == RT_FORMAT_DTA_114)
        return "dta114";
    if (format == RT_FORMAT_DTA_117)
        return "dta117";
    if (format == RT_FORMAT_DTA_118)
        return "dta118";
    if (format == RT_FORMAT_DTA_119)
        return "dta119";
    if (format == RT_FORMAT_SAV_COMP_NONE)
        return "sav";
    if (format == RT_FORMAT_SAV_COMP_ROWS)
        return "savrow";
    if (format == RT_FORMAT_SAV_COMP_ZLIB)
        return "zsav";
    if (format == RT_FORMAT_POR)
        return "por";
    if (format == RT_FORMAT_SAS7BCAT)
        return "sas7bcat";
    if (format == RT_FORMAT_SAS7BDAT_32BIT_COMP_NONE)
        return "sas7bdat32";
    if (format == RT_FORMAT_SAS7BDAT_32BIT_COMP_ROWS)
        return "sas7bdat32row";
    if (format == RT_FORMAT_SAS7BDAT_64BIT_COMP_NONE)
        return "sas7bdat64";
    if (format == RT_FORMAT_SAS7BDAT_64BIT_COMP_ROWS)
        return "sas7bdat64row";
    if (format == RT_FORMAT_XPORT_5)
        return "xpt5";
    if (format == RT_FORMAT_XPORT_8)
        return "xpt8";

    return "data";
}

rt_parse_ctx_t *parse_ctx_init(rt_buffer_t *buffer, rt_test_file_t *file, rt_test_args_t *args) {
    rt_parse_ctx_t *parse_ctx = calloc(1, sizeof(rt_parse_ctx_t));
    parse_ctx->buffer_ctx = buffer_ctx_init(buffer);
    parse_ctx->file = file;
    parse_ctx->args = args;
    return parse_ctx;
}

void parse_ctx_reset(rt_parse_ctx_t *parse_ctx, long file_format) {
    parse_ctx->file_format = file_format;
    parse_ctx->file_extension = file_extension(file_format);
    if ((file_format & RT_FORMAT_DTA_118_AND_NEWER)) {
        parse_ctx->max_file_label_len = 321;
    } else if ((file_format & RT_FORMAT_DTA_105_AND_OLDER)) {
        parse_ctx->max_file_label_len = 32;
    } else if ((file_format & RT_FORMAT_DTA)) {
        parse_ctx->max_file_label_len = 81;
    } else if ((file_format & RT_FORMAT_SAV)) {
        parse_ctx->max_file_label_len = 64;
    } else if ((file_format & RT_FORMAT_SAS7BDAT)) {
        parse_ctx->max_table_name_len = 32;
        parse_ctx->max_file_label_len = 256;
    } else {
        parse_ctx->max_file_label_len = 20;
    }
    if ((file_format & RT_FORMAT_XPORT_5)) {
        parse_ctx->max_table_name_len = 8;
    } else if ((file_format & RT_FORMAT_XPORT_8)) {
        parse_ctx->max_table_name_len = 32;
        parse_ctx->max_file_label_len = 256;
    }
    parse_ctx->var_index = -1;
    parse_ctx->obs_index = -1;
    parse_ctx->notes_count = 0;
    parse_ctx->variables_count = 0;
    parse_ctx->value_labels_count = 0;
    buffer_ctx_reset(parse_ctx->buffer_ctx);
}

void parse_ctx_free(rt_parse_ctx_t *parse_ctx) {
    if (parse_ctx->buffer_ctx) {
        free(parse_ctx->buffer_ctx);
    }
    free(parse_ctx);
}

long expected_row_count(rt_parse_ctx_t *parse_ctx) {
    long expected_rows = parse_ctx->file->rows;
    if (parse_ctx->args->row_offset > 0)
        expected_rows -= parse_ctx->args->row_offset;
    if (expected_rows < 0)
        expected_rows = 0;
    if (parse_ctx->args->row_limit > 0 && parse_ctx->args->row_limit < expected_rows)
        expected_rows = parse_ctx->args->row_limit;
    return expected_rows;
}

static int handle_metadata(readstat_metadata_t *metadata, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;

    rt_ctx->var_index = -1;
    rt_ctx->obs_index = -1;

    int var_count = readstat_get_var_count(metadata);
    int obs_count = readstat_get_row_count(metadata);
    const char *file_label = readstat_get_file_label(metadata);
    const char *table_name = readstat_get_table_name(metadata);
    time_t timestamp = readstat_get_creation_time(metadata);
    long format_version = readstat_get_file_format_version(metadata);

    push_error_if_doubles_differ(rt_ctx, 
            rt_ctx->file->columns_count, var_count, 
            "Number of variables");

    if (obs_count != -1) {
        push_error_if_doubles_differ(rt_ctx, 
                expected_row_count(rt_ctx), obs_count, 
                "Number of observations");
    }

    push_error_if_strings_differ_n(rt_ctx, rt_ctx->file->label, file_label, 
            rt_ctx->max_file_label_len-1, "File labels");
    if (table_name == NULL || strcmp(table_name, "DATASET") != 0) {
        push_error_if_strings_differ_n(rt_ctx, rt_ctx->file->table_name, table_name, 
                rt_ctx->max_table_name_len, "Table names");
    }
    if (rt_ctx->file->timestamp.tm_year) {
        struct tm timestamp_s = rt_ctx->file->timestamp;
        timestamp_s.tm_isdst = -1;
        push_error_if_doubles_differ(rt_ctx, mktime(&timestamp_s), timestamp, "File timestamps");
    }
    if (rt_ctx->file_format_version) {
        push_error_if_doubles_differ(rt_ctx, rt_ctx->file_format_version, 
                format_version, "Format versions");
    }

    return READSTAT_HANDLER_OK;
}

static int handle_note(int index, const char *note, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;
    push_error_if_strings_differ(rt_ctx, rt_ctx->file->notes[rt_ctx->notes_count++],
            note, "Note");

    return READSTAT_HANDLER_OK;
}

static int handle_fweight(readstat_variable_t *variable, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;
    int var_index = readstat_variable_get_index(variable);
    rt_column_t *column = &rt_ctx->file->columns[var_index];

    push_error_if_strings_differ(rt_ctx, rt_ctx->file->fweight,
            column->name, "Frequency weight");

    return READSTAT_HANDLER_OK;
}

static int handle_variable(int index, readstat_variable_t *variable,
                           const char *val_labels, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;
    rt_column_t *column = &rt_ctx->file->columns[index];

    rt_ctx->var_index = index;

    if (column->label_set[0])
        push_error_if_strings_differ(rt_ctx, column->label_set, 
                val_labels,
                "Column label sets");

    push_error_if_strings_differ(rt_ctx, column->name, 
            readstat_variable_get_name(variable),
            "Column names");

    push_error_if_strings_differ(rt_ctx, column->label,
            readstat_variable_get_label(variable),
            "Column labels");

    if (column->format[0])
        push_error_if_strings_differ(rt_ctx, column->format,
                readstat_variable_get_format(variable),
                "Column formats");

    if (column->display_width)
        push_error_if_doubles_differ(rt_ctx, column->display_width,
                readstat_variable_get_display_width(variable),
                "Column display widths");

    push_error_if_doubles_differ(rt_ctx, column->missing_ranges_count,
            readstat_variable_get_missing_ranges_count(variable),
            "Missing values count");

    long i;
    for (i=0; i<column->missing_ranges_count; i++) {
        push_error_if_values_differ(rt_ctx, column->missing_ranges[i].lo,
                readstat_variable_get_missing_range_lo(variable, i),
                "Missing range definition (lo value)");
        push_error_if_values_differ(rt_ctx, column->missing_ranges[i].hi,
                readstat_variable_get_missing_range_hi(variable, i),
                "Missing range definition (hi value)");
    }

    rt_ctx->variables_count++;

    return READSTAT_HANDLER_OK;
}

static int handle_value_label(const char *val_labels, readstat_value_t value, const char *label, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;
    long i, j;
    for (i=0; i<rt_ctx->file->label_sets_count; i++) {
        rt_label_set_t *label_set = &rt_ctx->file->label_sets[i];
        if (strcmp(val_labels, label_set->name) == 0) {
            for (j=0; j<label_set->value_labels_count; j++) {
                if (values_equal(value, label_set->value_labels[j].value)) {
                    push_error_if_strings_differ(rt_ctx, label_set->value_labels[j].label,
                            label, "Value label");
                    break;
                }
            }
            if (j == label_set->value_labels_count) {
                push_error_if_strings_differ(rt_ctx, NULL,
                        label, "Value label (no match)");
            }
            break;
        }
    }
    if (i == rt_ctx->file->label_sets_count) {
        push_error_if_strings_differ(rt_ctx, NULL,
                val_labels, "Label set");
    }
    rt_ctx->value_labels_count++;
    return READSTAT_HANDLER_OK;
}

static int handle_value(int obs_index, readstat_variable_t *variable, readstat_value_t value, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;
    rt_ctx->obs_index = obs_index;
    rt_ctx->var_index = readstat_variable_get_index(variable);
    long file_obs_index = obs_index + rt_ctx->args->row_offset;

    rt_column_t *column = &rt_ctx->file->columns[rt_ctx->var_index];

    if (column->type == READSTAT_TYPE_STRING_REF) {
        push_error_if_strings_differ(rt_ctx,
                rt_ctx->file->string_refs[readstat_int32_value(column->values[file_obs_index])],
                readstat_string_value(value), "String ref values");
    } else {
        push_error_if_values_differ(rt_ctx, 
                column->values[file_obs_index],
                value, "Data values");
    }

    return READSTAT_HANDLER_OK;
}

static void handle_error(const char *error_message, void *ctx) {
    printf("%s\n", error_message);
}

readstat_error_t read_file(rt_parse_ctx_t *parse_ctx, long format) {
    readstat_error_t error = READSTAT_OK;

    readstat_parser_t *parser = readstat_parser_init();

    readstat_set_open_handler(parser, rt_open_handler);
    readstat_set_close_handler(parser, rt_close_handler);
    readstat_set_seek_handler(parser, rt_seek_handler);
    readstat_set_read_handler(parser, rt_read_handler);
    readstat_set_update_handler(parser, rt_update_handler);
    readstat_set_io_ctx(parser, parse_ctx->buffer_ctx);

    readstat_set_metadata_handler(parser, &handle_metadata);
    readstat_set_note_handler(parser, &handle_note);
    readstat_set_variable_handler(parser, &handle_variable);
    readstat_set_fweight_handler(parser, &handle_fweight);
    readstat_set_value_handler(parser, &handle_value);
    readstat_set_value_label_handler(parser, &handle_value_label);
    readstat_set_error_handler(parser, &handle_error);

    readstat_set_row_limit(parser, parse_ctx->args->row_limit);
    readstat_set_row_offset(parser, parse_ctx->args->row_offset);

    if ((format & RT_FORMAT_DTA)) {
        parse_ctx->file_format_version = dta_file_format_version(format);
        error = readstat_parse_dta(parser, NULL, parse_ctx);
    } else if ((format & RT_FORMAT_SAV)) {
        parse_ctx->file_format_version = sav_file_format_version(format);
        error = readstat_parse_sav(parser, NULL, parse_ctx);
    } else if (format == RT_FORMAT_POR) {
        parse_ctx->file_format_version = 0;
        error = readstat_parse_por(parser, NULL, parse_ctx);
    } else if ((format & RT_FORMAT_SAS7BDAT)) {
        parse_ctx->file_format_version = sas_file_format_version(format);
        error = readstat_parse_sas7bdat(parser, NULL, parse_ctx);
    } else if ((format & RT_FORMAT_SAS7BCAT)) {
        error = readstat_parse_sas7bcat(parser, NULL, parse_ctx);
    } else if ((format & RT_FORMAT_XPORT)) {
        parse_ctx->file_format_version = sas_file_format_version(format);
        error = readstat_parse_xport(parser, NULL, parse_ctx);
    }
    if (error != READSTAT_OK)
        goto cleanup;

    push_error_if_doubles_differ(parse_ctx, parse_ctx->file->notes_count,
            parse_ctx->notes_count, "Note count");

    push_error_if_doubles_differ(parse_ctx, parse_ctx->file->columns_count,
            parse_ctx->variables_count, "Column count");

    push_error_if_doubles_differ(parse_ctx, expected_row_count(parse_ctx),
            parse_ctx->obs_index + 1, "Row count");

    long value_labels_count = 0;
    long i;
    for (i=0; i<parse_ctx->file->label_sets_count; i++) {
        value_labels_count += parse_ctx->file->label_sets[i].value_labels_count;
    }

    push_error_if_doubles_differ(parse_ctx, value_labels_count,
            parse_ctx->value_labels_count, "Value labels count");

cleanup:
    readstat_parser_free(parser);

    return error;
}

