#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <xlsxwriter.h>

#include "../../readstat.h"
#include "../module_util.h"
#include "../module.h"

#define MIN_ROWS_TO_SPLIT 35

typedef struct mod_xlsx_ctx_s {
    lxw_workbook *workbook;
    lxw_worksheet *worksheet;
    lxw_format *label_fmt;
    long row_count;
} mod_xlsx_ctx_t;

static int accept_file(const char *filename);
static void *ctx_init(const char *filename);
static void finish_file(void *ctx);
static int handle_info(int obs_count, int var_count, void *ctx);
static int handle_variable(int index, readstat_variable_t *variable,
                           const char *val_labels, void *ctx);
static int handle_value(int obs_index, int var_index, readstat_value_t value, void *ctx);

rs_module_t rs_mod_xlsx = {
    accept_file, /* accept */
    ctx_init, /* init */
    finish_file, /* finish */
    NULL, /* info */
    handle_variable,
    NULL, /* fweight */
    handle_value,
    NULL /* value label */
};

static int accept_file(const char *filename) {
    return rs_ends_with(filename, ".xlsx");
}

static void *ctx_init(const char *filename) {
    mod_xlsx_ctx_t *mod_ctx = malloc(sizeof(mod_xlsx_ctx_t));
    mod_ctx->workbook = workbook_new(filename);
    mod_ctx->worksheet = workbook_add_worksheet(mod_ctx->workbook, "Data");
    mod_ctx->label_fmt = workbook_add_format(mod_ctx->workbook);
    format_set_bold(mod_ctx->label_fmt);
    format_set_align(mod_ctx->label_fmt, LXW_ALIGN_CENTER);
    return mod_ctx;
}

static void finish_file(void *ctx) {
    mod_xlsx_ctx_t *mod_ctx = (mod_xlsx_ctx_t *)ctx;
    if (mod_ctx) {
        if (mod_ctx->row_count > MIN_ROWS_TO_SPLIT) {
            worksheet_freeze_panes(mod_ctx->worksheet, 1, 0);
        }
        workbook_close(mod_ctx->workbook);
        free(mod_ctx);
    }
}

static int handle_variable(int index, readstat_variable_t *variable,
                           const char *val_labels, void *ctx) {
    mod_xlsx_ctx_t *mod_ctx = (mod_xlsx_ctx_t *)ctx;
    const char *name = readstat_variable_get_name(variable);
    worksheet_write_string(mod_ctx->worksheet, 0, index, name, mod_ctx->label_fmt);
    worksheet_set_column(mod_ctx->worksheet, index, index, 2 * LXW_DEF_COL_WIDTH, NULL);
    return 0;
}

static int handle_value(int obs_index, int var_index, readstat_value_t value, void *ctx) {
    mod_xlsx_ctx_t *mod_ctx = (mod_xlsx_ctx_t *)ctx;
    readstat_types_t type = readstat_value_type(value);
    if (var_index == 0) {
        mod_ctx->row_count++;
    }
    if (readstat_value_is_system_missing(value)) {
        worksheet_write_blank(mod_ctx->worksheet, obs_index+1, var_index, NULL);
    } else if (type == READSTAT_TYPE_STRING || type == READSTAT_TYPE_LONG_STRING) {
        worksheet_write_string(mod_ctx->worksheet, obs_index+1, var_index, readstat_string_value(value), NULL);
    } else if (type == READSTAT_TYPE_CHAR ||
            type == READSTAT_TYPE_INT16 ||
            type == READSTAT_TYPE_INT32 ||
            type == READSTAT_TYPE_FLOAT ||
            type == READSTAT_TYPE_DOUBLE) {
        worksheet_write_number(mod_ctx->worksheet, obs_index+1, var_index, readstat_double_value(value), NULL);
    }
    return 0;
}
