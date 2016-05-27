#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "../../readstat.h"
#include "../../CKHashTable.h"
#include "../module_util.h"
#include "../module.h"

typedef struct mod_readstat_ctx_s {
    readstat_writer_t *writer;
    ck_hash_table_t *label_set_dict;

    long fweight_index;
    long var_count;
    long row_count;

    int out_fd;
    int is_sav:1;
    int is_dta:1;
} mod_readstat_ctx_t;

static ssize_t write_data(const void *bytes, size_t len, void *ctx);

static int accept_file(const char *filename);
static void *ctx_init(const char *filename);
static void finish_file(void *ctx);

static int handle_fweight(int var_index, void *ctx);
static int handle_info(int obs_count, int var_count, void *ctx);
static int handle_value_label(const char *val_labels, readstat_value_t value,
                              const char *label, void *ctx);
static int handle_variable(int index, readstat_variable_t *variable,
                           const char *val_labels, void *ctx);
static int handle_value(int obs_index, int var_index, readstat_value_t value, void *ctx);

rs_module_t rs_mod_readstat = {
    accept_file, /* accept */
    ctx_init, /* init */
    finish_file, /* finish */
    handle_info,
    handle_variable,
    handle_fweight,
    handle_value,
    handle_value_label
};

static ssize_t write_data(const void *bytes, size_t len, void *ctx) {
    mod_readstat_ctx_t *mod_ctx = (mod_readstat_ctx_t *)ctx;
    return write(mod_ctx->out_fd, bytes, len);
}

static int accept_file(const char *filename) {
    return rs_ends_with(filename, ".dta") || rs_ends_with(filename, ".sav");
}

static void *ctx_init(const char *filename) {
    size_t len = strlen(filename);
    mod_readstat_ctx_t *mod_ctx = malloc(sizeof(mod_readstat_ctx_t));
    mod_ctx->label_set_dict = ck_hash_table_init(1024);
    mod_ctx->is_sav = rs_ends_with(filename, ".sav");
    mod_ctx->is_dta = rs_ends_with(filename, ".dta");
    mod_ctx->out_fd = open(filename, O_CREAT | O_WRONLY | O_EXCL, 0644);
    if (mod_ctx->out_fd == -1) {
        dprintf(STDERR_FILENO, "Error opening %s for writing: %s\n", filename, strerror(errno));
        return NULL;
    }

    mod_ctx->writer = readstat_writer_init();
    readstat_writer_set_file_label(mod_ctx->writer, "Created by ReadStat <https://github.com/WizardMac/ReadStat>");
    readstat_set_data_writer(mod_ctx->writer, &write_data);

    return mod_ctx;
}

void finish_file(void *ctx) {
    mod_readstat_ctx_t *mod_ctx = (mod_readstat_ctx_t *)ctx;
    if (mod_ctx) {
        if (mod_ctx->out_fd != -1)
            close(mod_ctx->out_fd);
        if (mod_ctx->label_set_dict)
            ck_hash_table_free(mod_ctx->label_set_dict);
        if (mod_ctx->writer)
            readstat_writer_free(mod_ctx->writer);
        free(mod_ctx);
    }
}

static int handle_fweight(int var_index, void *ctx) {
    mod_readstat_ctx_t *mod_ctx = (mod_readstat_ctx_t *)ctx;
    readstat_writer_t *writer = mod_ctx->writer;
    readstat_writer_set_fweight_variable(writer, readstat_get_variable(writer, var_index));
    return 0;
}

static int handle_info(int obs_count, int var_count, void *ctx) {
    mod_readstat_ctx_t *mod_ctx = (mod_readstat_ctx_t *)ctx;
    mod_ctx->var_count = var_count;
    mod_ctx->row_count = obs_count;
    return (var_count == 0 || obs_count == 0);
}

static int handle_value_label(const char *val_labels, readstat_value_t value,
                              const char *label, void *ctx) {
    mod_readstat_ctx_t *mod_ctx = (mod_readstat_ctx_t *)ctx;
    readstat_writer_t *writer = mod_ctx->writer;
    readstat_label_set_t *label_set = NULL;
    readstat_types_t type = readstat_value_type(value);

    label_set = (readstat_label_set_t *)ck_str_hash_lookup(val_labels, mod_ctx->label_set_dict);
    if (label_set == NULL) {
        label_set = readstat_add_label_set(writer, type, val_labels);
        ck_str_hash_insert(val_labels, label_set, mod_ctx->label_set_dict);
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
    mod_readstat_ctx_t *mod_ctx = (mod_readstat_ctx_t *)ctx;
    readstat_writer_t *writer = mod_ctx->writer;

    readstat_types_t type = readstat_variable_get_type(variable);
    const char *name = readstat_variable_get_name(variable);
    const char *label = readstat_variable_get_label(variable);
    char *format = NULL;
    size_t width = readstat_variable_get_width(variable);
    readstat_label_set_t *label_set = NULL;
    
    if (val_labels) {
        label_set = (readstat_label_set_t *)ck_str_hash_lookup(val_labels, mod_ctx->label_set_dict);
    }

    readstat_add_variable(writer, type, width, name, label, format, label_set);

    return 0;
}

static int handle_value(int obs_index, int var_index, readstat_value_t value, void *ctx) {
    mod_readstat_ctx_t *mod_ctx = (mod_readstat_ctx_t *)ctx;
    readstat_writer_t *writer = mod_ctx->writer;

    readstat_variable_t *variable = readstat_get_variable(writer, var_index);
    readstat_types_t type = readstat_value_type(value);
    readstat_error_t error = READSTAT_OK;

    if (var_index == 0) {
        if (obs_index == 0) {
            if (mod_ctx->is_sav) {
                error = readstat_begin_writing_sav(writer, mod_ctx, mod_ctx->row_count);
            } else if (mod_ctx->is_dta) {
                error = readstat_begin_writing_dta(writer, mod_ctx, mod_ctx->row_count);
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

    if (var_index == mod_ctx->var_count - 1) {
        error = readstat_end_row(writer);
        if (error != READSTAT_OK)
            goto cleanup;

        if (obs_index == mod_ctx->row_count - 1) {
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
