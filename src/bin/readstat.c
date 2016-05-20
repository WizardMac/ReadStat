#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "readstat.h"
#include "CKHashTable.h"

#define RS_VERSION_STRING  "1.0-prerelease"

#define RS_FORMAT_UNKNOWN       0x00
#define RS_FORMAT_DTA           0x01
#define RS_FORMAT_SAV           0x02
#define RS_FORMAT_POR           0x04
#define RS_FORMAT_SAS_DATA      0x08
#define RS_FORMAT_SAS_CATALOG   0x10

#define RS_FORMAT_CAN_READ      (RS_FORMAT_DTA | RS_FORMAT_SAV | RS_FORMAT_POR | RS_FORMAT_SAS_DATA)
#define RS_FORMAT_CAN_WRITE     (RS_FORMAT_DTA | RS_FORMAT_SAV)

typedef struct rs_ctx_s {
    readstat_writer_t *writer;
    ck_hash_table_t *label_set_dict;

    long fweight_index;
    long var_count;
    long row_count;

    int out_format;
    int out_fd;
} rs_ctx_t;

int format(char *filename) {
    size_t len = strlen(filename);
    if (len < sizeof(".dta")-1)
        return RS_FORMAT_UNKNOWN;

    if (strncmp(filename + len - 4, ".dta", 4) == 0)
        return RS_FORMAT_DTA;

    if (strncmp(filename + len - 4, ".sav", 4) == 0)
        return RS_FORMAT_SAV;

    if (strncmp(filename + len - 4, ".por", 4) == 0)
        return RS_FORMAT_POR;

    if (len < sizeof(".sas7bdat")-1)
        return RS_FORMAT_UNKNOWN;

    if (strncmp(filename + len - 9, ".sas7bdat", 9) == 0)
        return RS_FORMAT_SAS_DATA;

    if (strncmp(filename + len - 9, ".sas7bcat", 9) == 0)
        return RS_FORMAT_SAS_CATALOG;

    return RS_FORMAT_UNKNOWN;
}

int is_catalog(char *filename) {
    return (format(filename) == RS_FORMAT_SAS_CATALOG);
}

int can_read(char *filename) {
    return (format(filename) & RS_FORMAT_CAN_READ);
}

int can_write(char *filename) {
    return (format(filename) & RS_FORMAT_CAN_WRITE);
}

static ssize_t write_data(const void *bytes, size_t len, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    return write(rs_ctx->out_fd, bytes, len);
}

static void handle_error(const char *msg, void *ctx) {
    dprintf(STDERR_FILENO, "%s", msg);
}

static int handle_fweight(int var_index, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    readstat_writer_t *writer = rs_ctx->writer;
    readstat_writer_set_fweight_variable(writer, readstat_get_variable(writer, var_index));
    return 0;
}

static int handle_info(int obs_count, int var_count, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    rs_ctx->var_count = var_count;
    rs_ctx->row_count = obs_count;
    return (var_count == 0 || obs_count == 0);
}

static int handle_value_label(const char *val_labels, readstat_value_t value,
                              const char *label, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    readstat_writer_t *writer = rs_ctx->writer;
    readstat_label_set_t *label_set = NULL;
    readstat_types_t type = readstat_value_type(value);

    label_set = (readstat_label_set_t *)ck_str_hash_lookup(val_labels, rs_ctx->label_set_dict);
    if (label_set == NULL) {
        label_set = readstat_add_label_set(writer, type, val_labels);
        ck_str_hash_insert(val_labels, label_set, rs_ctx->label_set_dict);
    }

    if (type == READSTAT_TYPE_INT32) {
        readstat_label_int32_value(label_set, readstat_int32_value(value), label);
    } else if (type == READSTAT_TYPE_DOUBLE) {
        readstat_label_double_value(label_set, readstat_double_value(value), label);
    } else if (type == READSTAT_TYPE_STRING) {
        readstat_label_string_value(label_set, readstat_string_value(value), label);
    }

    return 0;
}

static int handle_variable(int index, readstat_variable_t *variable,
                           const char *val_labels, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    readstat_writer_t *writer = rs_ctx->writer;

    readstat_types_t type = readstat_variable_get_type(variable);
    const char *name = readstat_variable_get_name(variable);
    const char *label = readstat_variable_get_label(variable);
    char *format = NULL;
    size_t width = readstat_variable_get_width(variable);
    readstat_label_set_t *label_set = NULL;
    
    if (val_labels) {
        label_set = (readstat_label_set_t *)ck_str_hash_lookup(val_labels, rs_ctx->label_set_dict);
    }

    readstat_add_variable(writer, type, width, name, label, format, label_set);

    return 0;
}

static int handle_value(int obs_index, int var_index, readstat_value_t value, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    readstat_writer_t *writer = rs_ctx->writer;

    readstat_variable_t *variable = readstat_get_variable(writer, var_index);
    readstat_types_t type = readstat_value_type(value);
    readstat_error_t error = READSTAT_OK;

    if (var_index == 0) {
        if (obs_index == 0) {
            if (rs_ctx->out_format == RS_FORMAT_SAV) {
                error = readstat_begin_writing_sav(writer, rs_ctx, rs_ctx->row_count);
            } else if (rs_ctx->out_format == RS_FORMAT_DTA) {
                error = readstat_begin_writing_dta(writer, rs_ctx, rs_ctx->row_count);
            }
            if (error != READSTAT_OK)
                goto cleanup;
        }
        if (var_index == 0) {
            error = readstat_begin_row(writer);
            if (error != READSTAT_OK)
                goto cleanup;
        }
    }

    if (readstat_value_is_system_missing(value)) {
        error = readstat_insert_missing_value(writer, variable);
    } else if (type == READSTAT_TYPE_STRING || type == READSTAT_TYPE_LONG_STRING) {
        error = readstat_insert_string_value(writer, variable, readstat_string_value(value));
    } else if (type == READSTAT_TYPE_CHAR) {
        error = readstat_insert_char_value(writer, variable, readstat_char_value(value));
    } else if (type == READSTAT_TYPE_INT16) {
        error = readstat_insert_int16_value(writer, variable, readstat_int16_value(value));
    } else if (type == READSTAT_TYPE_INT32) {
        error = readstat_insert_int32_value(writer, variable, readstat_int32_value(value));
    } else if (type == READSTAT_TYPE_FLOAT) {
        error = readstat_insert_float_value(writer, variable, readstat_float_value(value));
    } else if (type == READSTAT_TYPE_DOUBLE) {
        error = readstat_insert_double_value(writer, variable, readstat_double_value(value));
    }
    if (error != READSTAT_OK)
        goto cleanup;

    if (var_index == rs_ctx->var_count - 1) {
        error = readstat_end_row(writer);
        if (error != READSTAT_OK)
            goto cleanup;

        if (obs_index == rs_ctx->row_count - 1) {
            error = readstat_end_writing(writer);
            if (error != READSTAT_OK)
                goto cleanup;
        }
    }

cleanup:
    if (error != READSTAT_OK)
        return 1;

    return 0;
}

readstat_error_t parse_file(readstat_parser_t *parser, const char *input_filename, int input_format, void *ctx) {
    readstat_error_t error = READSTAT_OK;

    if (input_format == RS_FORMAT_DTA) {
        error = readstat_parse_dta(parser, input_filename, ctx);
    } else if (input_format == RS_FORMAT_SAV) {
        error = readstat_parse_sav(parser, input_filename, ctx);
    } else if (input_format == RS_FORMAT_POR) {
        error = readstat_parse_por(parser, input_filename, ctx);
    } else if (input_format == RS_FORMAT_SAS_DATA) {
        error = readstat_parse_sas7bdat(parser, input_filename, ctx);
    } else if (input_format == RS_FORMAT_SAS_CATALOG) {
        error = readstat_parse_sas7bcat(parser, input_filename, ctx);
    }

    return error;
}

rs_ctx_t *ctx_init() {
    rs_ctx_t *ctx = malloc(sizeof(rs_ctx_t));
    ctx->label_set_dict = ck_hash_table_init(1024);
    return ctx;
}

void ctx_free(rs_ctx_t *rs_ctx) {
    ck_hash_table_free(rs_ctx->label_set_dict);
    free(rs_ctx);
}

void print_version() {
    dprintf(STDERR_FILENO, "ReadStat version " RS_VERSION_STRING "\n");
}

void print_usage(const char *cmd) {
    print_version();

    dprintf(STDERR_FILENO, "\n  Standard usage:\n");
    dprintf(STDERR_FILENO, "\n     %s input.(dta|por|sav|sas7bdat) output.(dta|sav)\n", cmd);
    dprintf(STDERR_FILENO, "\n  Usage if your value labels are stored in a separate SAS catalog file:\n");
    dprintf(STDERR_FILENO, "\n     %s input.sas7bdat catalog.sas7bcat output.(dta|sav)\n\n", cmd);
}

int main(int argc, char** argv) {
    struct timeval start_time, end_time;
    readstat_error_t error = READSTAT_OK;
    char *input_filename = NULL;
    char *catalog_filename = NULL;
    char *output_filename = NULL;

    if (argc == 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
        print_version();
        return 0;
    } else if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    } if (argc == 3) {
        if (!can_read(argv[1]) || !can_write(argv[2])) {
            print_usage(argv[0]);
            return 1;
        }
        input_filename = argv[1];
        output_filename = argv[2];
    } else if (argc == 4) {
        if (!can_read(argv[1]) || !is_catalog(argv[2]) || !can_write(argv[3])) {
            print_usage(argv[0]);
            return 1;
        }
        input_filename = argv[1];
        catalog_filename = argv[2];
        output_filename = argv[3];
    } else {
        print_usage(argv[0]);
        return 1;
    }

    int input_format = format(input_filename);
    int output_format = format(output_filename);

    gettimeofday(&start_time, NULL);

    int fd = open(output_filename, O_CREAT | O_WRONLY | O_EXCL, 0644);
    if (fd == -1) {
        dprintf(STDERR_FILENO, "Error opening %s for writing: %s\n", output_filename, strerror(errno));
        return 1;
    }

    readstat_parser_t *pass1_parser = readstat_parser_init();
    readstat_parser_t *pass2_parser = readstat_parser_init();
    readstat_writer_t *writer = readstat_writer_init();
    readstat_writer_set_file_label(writer, "Created by ReadStat <https://github.com/WizardMac/ReadStat>");

    rs_ctx_t *rs_ctx = ctx_init();

    rs_ctx->writer = writer;
    rs_ctx->out_fd = fd;
    rs_ctx->out_format = output_format;

    readstat_set_data_writer(writer, &write_data);

    // Pass 1 - Collect fweight and value labels
    readstat_set_error_handler(pass1_parser, &handle_error);
    readstat_set_info_handler(pass1_parser, &handle_info);
    readstat_set_value_label_handler(pass1_parser, &handle_value_label);
    readstat_set_fweight_handler(pass1_parser, &handle_fweight);

    if (catalog_filename) {
        error = parse_file(pass1_parser, catalog_filename, RS_FORMAT_SAS_CATALOG, rs_ctx);
    } else {
        error = parse_file(pass1_parser, input_filename, input_format, rs_ctx);
    }
    if (error != READSTAT_OK)
        goto cleanup;

    // Pass 2 - Parse full file
    readstat_set_error_handler(pass2_parser, &handle_error);
    readstat_set_info_handler(pass2_parser, &handle_info);
    readstat_set_variable_handler(pass2_parser, &handle_variable);
    readstat_set_value_handler(pass2_parser, &handle_value);

    error = parse_file(pass2_parser, input_filename, input_format, rs_ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    gettimeofday(&end_time, NULL);

    dprintf(STDERR_FILENO, "Converted %ld variables and %ld rows in %.2lf seconds\n",
            rs_ctx->var_count, rs_ctx->row_count, 
            (end_time.tv_sec + 1e-6 * end_time.tv_usec) -
            (start_time.tv_sec + 1e-6 * start_time.tv_usec));

cleanup:
    readstat_parser_free(pass1_parser);
    readstat_parser_free(pass2_parser);
    readstat_writer_free(writer);
    ctx_free(rs_ctx);

    close(fd);

    if (error != READSTAT_OK) {
        dprintf(STDERR_FILENO, "%s\n", readstat_error_message(error));
        unlink(output_filename);
        return 1;
    }

    return 0;
}
