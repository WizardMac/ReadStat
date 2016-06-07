#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "../../readstat.h"
#include "../module_util.h"
#include "../module.h"

typedef struct mod_csv_ctx_s {
    int out_fd;
    long var_count;
} mod_csv_ctx_t;

static int accept_file(const char *filename);
static void *ctx_init(const char *filename);
static void finish_file(void *ctx);
static int handle_info(int obs_count, int var_count, void *ctx);
static int handle_variable(int index, readstat_variable_t *variable,
                           const char *val_labels, void *ctx);
static int handle_value(int obs_index, int var_index, readstat_value_t value, void *ctx);

rs_module_t rs_mod_csv = {
    accept_file, /* accept */
    ctx_init, /* init */
    finish_file, /* finish */
    handle_info, /* info */
    handle_variable,
    NULL, /* fweight */
    handle_value,
    NULL /* value label */
};

static int accept_file(const char *filename) {
    return rs_ends_with(filename, ".csv");
}

static void *ctx_init(const char *filename) {
    mod_csv_ctx_t *mod_ctx = malloc(sizeof(mod_csv_ctx_t));
    mod_ctx->out_fd = open(filename, O_CREAT | O_WRONLY | O_EXCL, 0644);
    if (mod_ctx->out_fd == -1) {
        fprintf(stderr, "Error opening %s for writing: %s\n", filename, strerror(errno));
        return NULL;
    }
    return mod_ctx;
}

static void finish_file(void *ctx) {
    mod_csv_ctx_t *mod_ctx = (mod_csv_ctx_t *)ctx;
    if (mod_ctx) {
        if (mod_ctx->out_fd != -1)
            close(mod_ctx->out_fd);
    }
}

static int handle_info(int obs_count, int var_count, void *ctx) {
    mod_csv_ctx_t *mod_ctx = (mod_csv_ctx_t *)ctx;
    mod_ctx->var_count = var_count;
    return mod_ctx->var_count == 0;
}

static int handle_variable(int index, readstat_variable_t *variable,
                           const char *val_labels, void *ctx) {
    mod_csv_ctx_t *mod_ctx = (mod_csv_ctx_t *)ctx;
    const char *name = readstat_variable_get_name(variable);
    if (index > 0) {
        dprintf(mod_ctx->out_fd, ",\"%s\"", name);
    } else {
        dprintf(mod_ctx->out_fd, "\"%s\"", name);
    }
    if (index == mod_ctx->var_count - 1) {
        dprintf(mod_ctx->out_fd, "\n");
    }
    return 0;
}

static int handle_value(int obs_index, int var_index, readstat_value_t value, void *ctx) {
    mod_csv_ctx_t *mod_ctx = (mod_csv_ctx_t *)ctx;
    readstat_types_t type = readstat_value_type(value);
    if (var_index > 0) {
        dprintf(mod_ctx->out_fd, ",");
    }
    if (readstat_value_is_system_missing(value)) {
        /* void */
    } else if (type == READSTAT_TYPE_STRING || type == READSTAT_TYPE_LONG_STRING) {
        /* TODO escape */
        dprintf(mod_ctx->out_fd, "\"%s\"", readstat_string_value(value));
    } else if (type == READSTAT_TYPE_CHAR) {
        dprintf(mod_ctx->out_fd, "%hhd", readstat_char_value(value));
    } else if (type == READSTAT_TYPE_INT16) {
        dprintf(mod_ctx->out_fd, "%hd", readstat_int16_value(value));
    } else if (type == READSTAT_TYPE_INT32) {
        dprintf(mod_ctx->out_fd, "%d", readstat_int32_value(value));
    } else if (type == READSTAT_TYPE_FLOAT) {
        dprintf(mod_ctx->out_fd, "%f", readstat_float_value(value));
    } else if (type == READSTAT_TYPE_DOUBLE) {
        dprintf(mod_ctx->out_fd, "%lf", readstat_double_value(value));
    }
    if (var_index == mod_ctx->var_count - 1) {
        dprintf(mod_ctx->out_fd, "\n");
    }
    return 0;
}
