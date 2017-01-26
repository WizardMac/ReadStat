#include <stdlib.h>

#include "../readstat.h"

#include "test_types.h"
#include "test_error.h"
#include "test_buffer.h"
#include "test_readstat.h"
#include "test_read.h"
#include "test_dta.h"
#include "test_sas.h"

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
    if (format == RT_FORMAT_SAV_COMP_NONE)
        return "sav";
    if (format == RT_FORMAT_SAV_COMP_ROWS)
        return "savrow";
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

static rt_buffer_ctx_t *buffer_ctx_init(rt_buffer_t *buffer) {
    rt_buffer_ctx_t *buffer_ctx = calloc(1, sizeof(rt_buffer_ctx_t));
    buffer_ctx->buffer = buffer;
    return buffer_ctx;
}

static void buffer_ctx_reset(rt_buffer_ctx_t *buffer_ctx) {
    buffer_reset(buffer_ctx->buffer);
    buffer_ctx->pos = 0;
}

rt_parse_ctx_t *parse_ctx_init(rt_buffer_t *buffer, rt_test_file_t *file) {
    rt_parse_ctx_t *parse_ctx = calloc(1, sizeof(rt_parse_ctx_t));
    parse_ctx->buffer_ctx = buffer_ctx_init(buffer);
    parse_ctx->file = file;
    return parse_ctx;
}

void parse_ctx_reset(rt_parse_ctx_t *parse_ctx, long file_format) {
    parse_ctx->file_format = file_format;
    parse_ctx->file_extension = file_extension(file_format);
    if ((file_format & RT_FORMAT_DTA_118)) {
        parse_ctx->max_file_label_len = 321;
    } else if ((file_format & RT_FORMAT_DTA_105_AND_OLDER)) {
        parse_ctx->max_file_label_len = 32;
    } else if ((file_format & RT_FORMAT_DTA)) {
        parse_ctx->max_file_label_len = 81;
    } else if ((file_format & RT_FORMAT_SAV)) {
        parse_ctx->max_file_label_len = 64;
    } else if ((file_format & RT_FORMAT_SAS7BDAT)) {
        parse_ctx->max_file_label_len = 64;
    } else {
        parse_ctx->max_file_label_len = 20;
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

static int rt_open_handler(const char *path, void *io_ctx) {
    return 0;
}

static int rt_close_handler(void *io_ctx) {
    return 0;
}

static readstat_off_t rt_seek_handler(readstat_off_t offset,
        readstat_io_flags_t whence, void *io_ctx) {
    rt_buffer_ctx_t *buffer_ctx = (rt_buffer_ctx_t *)io_ctx;
    readstat_off_t newpos = -1;
    if (whence == READSTAT_SEEK_SET) {
        newpos = offset;
    } else if (whence == READSTAT_SEEK_CUR) {
        newpos = buffer_ctx->pos + offset;
    } else if (whence == READSTAT_SEEK_END) {
        newpos = buffer_ctx->buffer->used + offset;
    }

    if (newpos < 0)
        return -1;

    if (newpos > buffer_ctx->buffer->used)
        return -1;

    buffer_ctx->pos = newpos;
    return newpos;
}

static ssize_t rt_read_handler(void *buf, size_t nbytes, void *io_ctx) {
    rt_buffer_ctx_t *buffer_ctx = (rt_buffer_ctx_t *)io_ctx;
    ssize_t bytes_copied = 0;
    ssize_t bytes_left = buffer_ctx->buffer->used - buffer_ctx->pos;
    if (nbytes <= bytes_left) {
        memcpy(buf, buffer_ctx->buffer->bytes + buffer_ctx->pos, nbytes);
        bytes_copied = nbytes;
    } else if (bytes_left > 0) {
        memcpy(buf, buffer_ctx->buffer->bytes + buffer_ctx->pos, bytes_left);
        bytes_copied = bytes_left;
    }
    buffer_ctx->pos += bytes_copied;
    return bytes_copied;
}

static readstat_error_t rt_update_handler(long file_size,
        readstat_progress_handler progress_handler, void *user_ctx,
        void *io_ctx) {
    if (!progress_handler)
        return READSTAT_OK;

    rt_buffer_ctx_t *buffer_ctx = (rt_buffer_ctx_t *)io_ctx;

    if (progress_handler(1.0 * buffer_ctx->pos / buffer_ctx->buffer->used, user_ctx))
        return READSTAT_ERROR_USER_ABORT;

    return READSTAT_OK;
}

static int handle_info(int obs_count, int var_count, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;

    rt_ctx->var_index = -1;
    rt_ctx->obs_index = -1;

    push_error_if_doubles_differ(rt_ctx, 
            rt_ctx->file->columns_count, var_count, 
            "Number of variables");

    if (obs_count != -1) {
        push_error_if_doubles_differ(rt_ctx, 
                rt_ctx->file->rows, obs_count, 
                "Number of observations");
    }

    return READSTAT_HANDLER_OK;
}

static int handle_metadata(const char *file_label, time_t timestamp, long format_version, void *ctx) {
    rt_parse_ctx_t *rt_ctx = (rt_parse_ctx_t *)ctx;

    push_error_if_strings_differ_n(rt_ctx, rt_ctx->file->label, file_label, 
            rt_ctx->max_file_label_len, "File labels");
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

    rt_column_t *column = &rt_ctx->file->columns[rt_ctx->var_index];

    if (column->type == READSTAT_TYPE_STRING_REF) {
        push_error_if_strings_differ(rt_ctx,
                rt_ctx->file->string_refs[readstat_int32_value(column->values[obs_index])],
                readstat_string_value(value), "String ref values");
    } else {
        push_error_if_values_differ(rt_ctx, 
                column->values[obs_index],
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

    readstat_set_info_handler(parser, &handle_info);
    readstat_set_metadata_handler(parser, &handle_metadata);
    readstat_set_note_handler(parser, &handle_note);
    readstat_set_variable_handler(parser, &handle_variable);
    readstat_set_fweight_handler(parser, &handle_fweight);
    readstat_set_value_handler(parser, &handle_value);
    readstat_set_value_label_handler(parser, &handle_value_label);
    readstat_set_error_handler(parser, &handle_error);

    if ((format & RT_FORMAT_DTA)) {
        parse_ctx->file_format_version = dta_file_format_version(format);
        error = readstat_parse_dta(parser, NULL, parse_ctx);
    } else if ((format & RT_FORMAT_SAV)) {
        parse_ctx->file_format_version = 2;
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

    push_error_if_doubles_differ(parse_ctx, parse_ctx->file->rows,
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

