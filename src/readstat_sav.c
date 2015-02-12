//
//  sav.c
//  Wizard
//
//  Created by Evan Miller on 12/16/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <time.h>

#include "readstat_io.h"
#include "readstat_sav.h"
#include "readstat_sav_parse.h"
#include "readstat_spss_parse.h"
#include "readstat_convert.h"
#include "readstat_writer.h"

#define READSTAT_PRODUCT_NAME       "ReadStat"
#define READSTAT_PRODUCT_URL        "https://github.com/WizardMac/ReadStat"

#define MAX_TEXT_SIZE               256
#define DATA_BUFFER_SIZE            65536

/* Others defined in table below */

/* See http://msdn.microsoft.com/en-us/library/dd317756(VS.85).aspx */
static readstat_charset_entry_t _charset_table[] = { 
    { .code = 1,     .name = "EBCDIC-US" },
    { .code = 2,     .name = "US-ASCII" },
    { .code = 3,     .name = "WINDOWS-1252" },
    { .code = 4,     .name = "DEC-KANJI" },
    { .code = 437,   .name = "CP437" },
    { .code = 708,   .name = "ASMO-708" },
    { .code = 737,   .name = "CP737" },
    { .code = 775,   .name = "CP775" },
    { .code = 850,   .name = "CP850" },
    { .code = 852,   .name = "CP852" },
    { .code = 855,   .name = "CP855" },
    { .code = 857,   .name = "CP857" },
    { .code = 858,   .name = "CP858" },
    { .code = 860,   .name = "CP860" },
    { .code = 861,   .name = "CP861" },
    { .code = 862,   .name = "CP862" },
    { .code = 863,   .name = "CP863" },
    { .code = 864,   .name = "CP864" },
    { .code = 865,   .name = "CP865" },
    { .code = 866,   .name = "CP866" },
    { .code = 869,   .name = "CP869" },
    { .code = 874,   .name = "CP874" },
    { .code = 932,   .name = "SHIFT-JIS" },
    { .code = 936,   .name = "ISO-IR-58" },
    { .code = 949,   .name = "ISO-IR-149" },
    { .code = 950,   .name = "BIG-5" },
    { .code = 1200,  .name = "UTF-16LE" },
    { .code = 1201,  .name = "UTF-16BE" },
    { .code = 1250,  .name = "WINDOWS-1250" },
    { .code = 1251,  .name = "WINDOWS-1251" },
    { .code = 1252,  .name = "WINDOWS-1252" },
    { .code = 1253,  .name = "WINDOWS-1253" },
    { .code = 1254,  .name = "WINDOWS-1254" },
    { .code = 1255,  .name = "WINDOWS-1255" },
    { .code = 1256,  .name = "WINDOWS-1256" },
    { .code = 1257,  .name = "WINDOWS-1257" },
    { .code = 1258,  .name = "WINDOWS-1258" },
    { .code = 1361,  .name = "CP1361" },
    { .code = 10000, .name = "MACROMAN" },
    { .code = 10004, .name = "MACARABIC" },
    { .code = 10005, .name = "MACHEBREW" },
    { .code = 10006, .name = "MACGREEK" },
    { .code = 10007, .name = "MACCYRILLIC" },
    { .code = 10010, .name = "MACROMANIA" },
    { .code = 10017, .name = "MACUKRAINE" },
    { .code = 10021, .name = "MACTHAI" },
    { .code = 10029, .name = "MACCENTRALEUROPE" },
    { .code = 10079, .name = "MACICELAND" },
    { .code = 10081, .name = "MACTURKISH" },
    { .code = 10082, .name = "MACCROATIAN" },
    { .code = 12000, .name = "UTF-32LE" },
    { .code = 12001, .name = "UTF-32BE" },
    { .code = 20127, .name = "US-ASCII" },
    { .code = 20866, .name = "KOI8-R" },
    { .code = 20932, .name = "EUC-JP" },
    { .code = 21866, .name = "KOI8-U" },
    { .code = 28591, .name = "ISO-8859-1" },
    { .code = 28592, .name = "ISO-8859-2" },
    { .code = 28593, .name = "ISO-8859-3" },
    { .code = 28594, .name = "ISO-8859-4" },
    { .code = 28595, .name = "ISO-8859-5" },
    { .code = 28596, .name = "ISO-8859-6" },
    { .code = 28597, .name = "ISO-8859-7" },
    { .code = 28598, .name = "ISO-8859-8" },
    { .code = 28599, .name = "ISO-8859-9" },
    { .code = 28603, .name = "ISO-8859-13" },
    { .code = 28605, .name = "ISO-8859-15" },
    { .code = 50220, .name = "ISO-2022-JP" },
    { .code = 50221, .name = "ISO-2022-JP" }, // same as above?
    { .code = 50222, .name = "ISO-2022-JP" }, // same as above?
    { .code = 50225, .name = "ISO-2022-KR" },
    { .code = 50229, .name = "ISO-2022-CN" },
    { .code = 51932, .name = "EUC-JP" },
    { .code = 51936, .name = "EUC-CN" },
    { .code = 51949, .name = "EUC-KR" },
    { .code = 52936, .name = "HZ-GB-2312" },
    { .code = 54936, .name = "GB18030" },
    { .code = 65000, .name = "UTF-7" },
    { .code = 65001, .name = "UTF-8" }
};

#define SAV_VARINFO_INITIAL_CAPACITY  512
#define SAV_EIGHT_SPACES              "        "
#define SAV_LABEL_NAME_PREFIX         "labels"

static void sav_ctx_free(sav_ctx_t *ctx);
static readstat_error_t sav_read_data(int fd, sav_ctx_t *ctx, void *user_ctx);
static readstat_error_t sav_read_compressed_data(int fd, size_t longest_string, 
        sav_ctx_t *ctx, void *user_ctx, int *out_rows);
static readstat_error_t sav_read_uncompressed_data(int fd, size_t longest_string, 
        sav_ctx_t *ctx, void *user_ctx, int *out_rows);
static readstat_error_t sav_read_variable_record(int fd, sav_ctx_t *ctx);
static readstat_error_t sav_read_document_record(int fd, sav_ctx_t *ctx);
static readstat_error_t sav_read_value_label_record(int fd, sav_ctx_t *ctx, void *user_ctx);
static readstat_error_t sav_read_dictionary_termination_record(int fd, sav_ctx_t *ctx);
static readstat_error_t sav_parse_machine_floating_point_record(void *data, sav_ctx_t *ctx);
static readstat_error_t sav_parse_variable_display_parameter_record(void *data, sav_ctx_t *ctx);
static readstat_error_t sav_parse_machine_integer_info_record(void *data, size_t data_len, sav_ctx_t *ctx);

sav_ctx_t *sav_ctx_init(sav_file_header_record_t *header) {
    sav_ctx_t *ctx = NULL;
    if ((ctx = malloc(sizeof(sav_ctx_t))) == NULL) {
        return NULL;
    }
    memset(ctx, 0, sizeof(sav_ctx_t));
    
    if (header->layout_code == 2 || header->layout_code == 3) {
        ctx->machine_needs_byte_swap = 0;
    } else {
        ctx->machine_needs_byte_swap = 1;
    }
    
    ctx->data_is_compressed = (header->compressed != 0);
    ctx->record_count = ctx->machine_needs_byte_swap ? byteswap4(header->ncases) : header->ncases;
    
    double bias = ctx->machine_needs_byte_swap ? byteswap_double(header->bias) : header->bias;
    
    if (bias != 100.0) {
        sav_ctx_free(ctx);
        return NULL;
    }
    
    ctx->varinfo_capacity = SAV_VARINFO_INITIAL_CAPACITY;
    
    if ((ctx->varinfo = calloc(ctx->varinfo_capacity, sizeof(sav_varinfo_t))) == NULL) {
        sav_ctx_free(ctx);
        return NULL;
    }
    
    return ctx;
}

static void sav_ctx_free(sav_ctx_t *ctx) {
    if (ctx->varinfo) {
        int i;
        for (i=0; i<ctx->var_count; i++) {
            free(ctx->varinfo[i].label);
        }
        free(ctx->varinfo);
    }
    if (ctx->converter) {
        iconv_close(ctx->converter);
    }
    free(ctx);
}

static readstat_error_t sav_read_variable_record(int fd, sav_ctx_t *ctx) {
    sav_variable_record_t variable;
    readstat_error_t retval = READSTAT_OK;
    if (ctx->var_index == ctx->varinfo_capacity) {
        if ((ctx->varinfo = realloc(ctx->varinfo, (ctx->varinfo_capacity *= 2) * sizeof(sav_varinfo_t))) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
    }
    if (read(fd, &variable, sizeof(sav_variable_record_t)) < sizeof(sav_variable_record_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    variable.print = ctx->machine_needs_byte_swap ? byteswap4(variable.print) : variable.print;
    variable.write = ctx->machine_needs_byte_swap ? byteswap4(variable.write) : variable.write;

    readstat_types_t dta_type = READSTAT_TYPE_DOUBLE;
    int32_t type = ctx->machine_needs_byte_swap ? byteswap4(variable.type) : variable.type;
    int i;
    if (type < 0) {
        if (ctx->var_index == 0) {
            return READSTAT_ERROR_PARSE;
        }
        ctx->var_offset++;
        sav_varinfo_t *prev = &ctx->varinfo[ctx->var_index-1];
        prev->width++;
        return 0;
    }
    if (type > 0) {
        dta_type = READSTAT_TYPE_STRING;
        // len = type;
    }
    sav_varinfo_t *info = &ctx->varinfo[ctx->var_index];
    memset(info, 0, sizeof(sav_varinfo_t));
    info->width = 1;
    info->index = ctx->var_index;
    info->offset = ctx->var_offset;
    info->type = dta_type;

    retval = readstat_convert(info->name, sizeof(info->name), variable.name, 8, ctx->converter);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = readstat_convert(info->longname, sizeof(info->longname), variable.name, 8, ctx->converter);
    if (retval != READSTAT_OK)
        goto cleanup;

    info->print_format.decimal_places = (variable.print & 0x000000FF);
    info->print_format.width = (variable.print & 0x0000FF00) >> 8;
    info->print_format.type = (variable.print  & 0x00FF0000) >> 16;

    info->write_format.decimal_places = (variable.write & 0x000000FF);
    info->write_format.width = (variable.write & 0x0000FF00) >> 8;
    info->write_format.type = (variable.write  & 0x00FF0000) >> 16;
    
    if (variable.has_var_label) {
        int32_t label_len;
        if (read(fd, &label_len, sizeof(int32_t)) < sizeof(int32_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        label_len = ctx->machine_needs_byte_swap ? byteswap4(label_len) : label_len;
        int32_t label_capacity = (label_len + 3) / 4 * 4;
        char *label_buf = malloc(label_capacity);
        size_t out_label_len = label_len*4+1;
        info->label = malloc(out_label_len);
        if (label_buf == NULL || info->label == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        if (read(fd, label_buf, label_capacity) < label_capacity) {
            retval = READSTAT_ERROR_READ;
            free(label_buf);
            free(info->label);
            info->label = NULL;
            goto cleanup;
        }
        retval = readstat_convert(info->label, out_label_len, label_buf, label_len, ctx->converter);
        free(label_buf);
        if (retval != READSTAT_OK)
            goto cleanup;
    }
    
    ctx->varinfo[ctx->var_index].labels_index = -1;
    
    if (variable.n_missing_values) {
        info->n_missing_values = ctx->machine_needs_byte_swap ? byteswap4(variable.n_missing_values) : variable.n_missing_values;
        if (info->n_missing_values < 0) {
            info->missing_range = 1;
            info->n_missing_values = abs(info->n_missing_values);
        } else {
            info->missing_range = 0;
        }
        if (info->n_missing_values > 3) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
        if ((read(fd, info->missing_values, info->n_missing_values * sizeof(double))) < info->n_missing_values * sizeof(double)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        if (ctx->machine_needs_byte_swap) {
            for (i=0; i<info->n_missing_values; i++) {
                info->missing_values[i] = byteswap_double(info->missing_values[i]);
            }
        }
    }
    
    ctx->var_index++;
    ctx->var_offset++;
    
cleanup:
    
    return retval;
}

static int sav_varinfo_compare(const void *elem1, const void *elem2) {
    int offset = *(int *)elem1;
    const sav_varinfo_t *v = (const sav_varinfo_t *)elem2;
    if (offset < v->offset)
        return -1;
    return (offset > v->offset);
}

static readstat_error_t sav_read_value_label_record(int fd, sav_ctx_t *ctx, void *user_ctx) {
    int32_t label_count;
    readstat_error_t retval = READSTAT_OK;
    int32_t *vars = NULL;
    int32_t rec_type;
    int32_t var_count;
    readstat_types_t value_type = READSTAT_TYPE_STRING;
    char label_name_buf[256];
    char label_buf[256];
    typedef struct value_label_s {
        char          value[8];
        unsigned char label_len;
        char          label[256*4+1];
    } value_label_t;
    value_label_t *value_labels = NULL;

    if (read(fd, &label_count, sizeof(int32_t)) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        label_count = byteswap4(label_count);
    
    if ((value_labels = malloc(label_count * sizeof(value_label_t))) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    snprintf(label_name_buf, sizeof(label_name_buf), SAV_LABEL_NAME_PREFIX "%d", ctx->value_labels_count);
    int i;
    for (i=0; i<label_count; i++) {
        value_label_t *vlabel = &value_labels[i];
        if (read(fd, vlabel, 9) < 9) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        size_t label_len = (vlabel->label_len + 8) / 8 * 8 - 1;
        if (read(fd, label_buf, label_len) < label_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        retval = readstat_convert(vlabel->label, sizeof(vlabel->label), label_buf, label_len, ctx->converter);
        if (retval != READSTAT_OK)
            goto cleanup;
    }

    if (read(fd, &rec_type, sizeof(int32_t)) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        rec_type = byteswap4(rec_type);
    
    if (rec_type != 4) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (read(fd, &var_count, sizeof(int32_t)) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        var_count = byteswap4(var_count);
    
    if ((vars = malloc(var_count * sizeof(int32_t))) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    if (read(fd, vars, var_count * sizeof(int32_t)) < var_count * sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    for (i=0; i<var_count; i++) {
        int var_offset = vars[i]-1; // Why subtract 1????
        sav_varinfo_t *var = bsearch(&var_offset, ctx->varinfo, ctx->var_index, sizeof(sav_varinfo_t),
                &sav_varinfo_compare);
        if (var) {
            value_type = var->type;
            var->labels_index = ctx->value_labels_count;
        }
    }
    for (i=0; i<label_count; i++) {
        value_label_t *vlabel = &value_labels[i];
        if (value_type == READSTAT_TYPE_DOUBLE) {
            double val_d = 0.0;
            memcpy(&val_d, vlabel->value, 8);
            if (ctx->machine_needs_byte_swap)
                val_d = byteswap_double(val_d);
            ctx->value_label_handler(label_name_buf, &val_d, value_type, vlabel->label, user_ctx);
        } else {
            char unpadded_val[8*4+1];
            retval = readstat_convert(unpadded_val, sizeof(unpadded_val), vlabel->value, 8, ctx->converter);
            if (retval != READSTAT_OK)
                break;
            ctx->value_label_handler(label_name_buf, unpadded_val, value_type, vlabel->label, user_ctx);
        }
    }
    ctx->value_labels_count++;
cleanup:
    if (vars)
        free(vars);
    if (value_labels)
        free(value_labels);
    
    return retval;
}

static readstat_error_t sav_read_document_record(int fd, sav_ctx_t *ctx) {
    int32_t n_lines;
    readstat_error_t retval = READSTAT_OK;
    if (read(fd, &n_lines, sizeof(int32_t)) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        n_lines = byteswap4(n_lines);
    if (lseek(fd, n_lines * 80, SEEK_CUR) == -1) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
cleanup:
    return retval;
}

static readstat_error_t sav_read_dictionary_termination_record(int fd, sav_ctx_t *ctx) {
    int32_t filler;
    readstat_error_t retval = READSTAT_OK;
    if (read(fd, &filler, sizeof(int32_t)) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
    }
    return retval;
}

double handle_missing_double(double fp_value, sav_varinfo_t *info) {
    if (info->missing_range) {
        if (fp_value >= info->missing_values[0] && fp_value <= info->missing_values[1]) {
            return NAN;
        }
    } else {
        if (info->n_missing_values > 0 && fp_value == info->missing_values[0]) {
            return NAN;
        } else if (info->n_missing_values > 1 && fp_value == info->missing_values[1]) {
            return NAN;
        }
    }
    if (info->n_missing_values == 3 && fp_value == info->missing_values[2]) {
        return NAN;
    }
    uint64_t long_value = 0;
    memcpy(&long_value, &fp_value, 8);
    if (long_value == SAV_MISSING_DOUBLE)
        return NAN;
    if (long_value == SAV_LOWEST_DOUBLE)
        return NAN;
    if (long_value == SAV_HIGHEST_DOUBLE)
        return NAN;
    
    return fp_value;
}

static readstat_error_t sav_read_data(int fd, sav_ctx_t *ctx, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    int longest_string = 256;
    int rows = 0;
    int i;

    for (i=0; i<ctx->var_count; i++) {
        sav_varinfo_t *info = &ctx->varinfo[i];
        if (info->string_length > longest_string) {
            longest_string = info->string_length;
        }
    }
    if (ctx->data_is_compressed) {
        retval = sav_read_compressed_data(fd, longest_string, ctx, user_ctx, &rows);
    } else {
        retval = sav_read_uncompressed_data(fd, longest_string, ctx, user_ctx, &rows);
    }
    if (retval != READSTAT_OK)
        goto done;

    if (rows != ctx->record_count) {
        retval = READSTAT_ERROR_PARSE;
    }
done:
    
    return retval;
}

static readstat_error_t sav_read_uncompressed_data(int fd, size_t longest_string, 
        sav_ctx_t *ctx, void *user_ctx, int *out_rows) {
    readstat_error_t retval = READSTAT_OK;
    int segment_offset = 0;
    int row = 0, var_index = 0, col = 0;
    double fp_value;
    int offset = 0;
    off_t data_offset = 0;
    int raw_str_used = 0;
    char *raw_str_value = NULL;
    char *utf8_str_value = NULL;
    size_t utf8_str_value_len = 0;
    unsigned char buffer[DATA_BUFFER_SIZE];
    int buffer_used = 0;

    if ((raw_str_value = malloc(longest_string)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto done;
    }
    utf8_str_value_len = longest_string*4+1;
    if ((utf8_str_value = malloc(utf8_str_value_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto done;
    }
    while (1) {
        if (data_offset >= buffer_used) {
            if ((buffer_used = read(fd, buffer, sizeof(buffer))) == -1 ||
                buffer_used == 0 || (buffer_used % 8) != 0)
                goto done;

            data_offset = 0;
        }

        sav_varinfo_t *col_info = &ctx->varinfo[col];
        sav_varinfo_t *var_info = &ctx->varinfo[var_index];
        if (offset > 31) {
            retval = READSTAT_ERROR_PARSE;
            goto done;
        }
        if (var_info->type == READSTAT_TYPE_STRING) {
            if (raw_str_used + 8 <= longest_string) {
                memcpy(raw_str_value + raw_str_used, &buffer[data_offset], 8);
                raw_str_used += 8;
            }
            offset++;
            if (offset == col_info->width) {
                segment_offset++;
                if (segment_offset == var_info->n_segments) {
                    retval = readstat_convert(utf8_str_value, utf8_str_value_len, 
                            raw_str_value, raw_str_used, ctx->converter);
                    if (retval != READSTAT_OK)
                        goto done;
                    if (ctx->value_handler(row, var_info->index, utf8_str_value, READSTAT_TYPE_STRING, user_ctx)) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        goto done;
                    }
                    raw_str_used = 0;
                    segment_offset = 0;
                    var_index += var_info->n_segments;
                }
                offset = 0;
                col++;
            }
        } else if (var_info->type == READSTAT_TYPE_DOUBLE) {
            memcpy(&fp_value, &buffer[data_offset], 8);
            if (ctx->machine_needs_byte_swap) {
                fp_value = byteswap_double(fp_value);
            }
            fp_value = handle_missing_double(fp_value, var_info);
            if (ctx->value_handler(row, var_info->index, isnan(fp_value) ? NULL : &fp_value, READSTAT_TYPE_DOUBLE, user_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto done;
            }
            var_index += var_info->n_segments;
            col++;
        }
        if (col == ctx->var_index) {
            col = 0;
            var_index = 0;
            row++;
        }
        data_offset += 8;
    }
done:
    if (retval == READSTAT_OK) {
        if (out_rows)
            *out_rows = row;
    }
    if (raw_str_value)
        free(raw_str_value);
    if (utf8_str_value)
        free(utf8_str_value);

    return retval;
}

static readstat_error_t sav_read_compressed_data(int fd,
        size_t longest_string, sav_ctx_t *ctx, void *user_ctx, int *out_rows) {
    readstat_error_t retval = READSTAT_OK;
    unsigned char value[8];
    int offset = 0;
    int segment_offset = 0;
    int row = 0, var_index = 0, col = 0;
    int i;
    double fp_value;
    off_t data_offset = 0;
    int raw_str_used = 0;
    char *raw_str_value = NULL;
    char *utf8_str_value = NULL;
    size_t utf8_str_value_len = 0;
    unsigned char buffer[DATA_BUFFER_SIZE];
    int buffer_used = 0;

    if ((raw_str_value = malloc(longest_string)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto done;
    }
    utf8_str_value_len = longest_string*4+1;
    if ((utf8_str_value = malloc(utf8_str_value_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto done;
    }
    while (1) {
        if (data_offset >= buffer_used) {
            if ((buffer_used = read(fd, buffer, sizeof(buffer))) == -1 ||
                buffer_used == 0 || (buffer_used % 8) != 0)
                goto done;

            data_offset = 0;
        }

        memcpy(value, &buffer[data_offset], 8);
        data_offset += 8;
        
        sav_varinfo_t *col_info = &ctx->varinfo[col];
        sav_varinfo_t *var_info = &ctx->varinfo[var_index];
        for (i=0; i<8; i++) {
            if (offset > 31) {
                retval = READSTAT_ERROR_PARSE;
                goto done;
            }
            col_info = &ctx->varinfo[col];
            var_info = &ctx->varinfo[var_index];
            switch (value[i]) {
                case 0:
                    break;
                case 252:
                    goto done;
                case 253:
                    if (data_offset >= buffer_used) {
                        if ((buffer_used = read(fd, buffer, sizeof(buffer))) == -1 ||
                            buffer_used == 0 || (buffer_used % 8) != 0)
                            goto done;

                        data_offset = 0;
                    }
                    if (var_info->type == READSTAT_TYPE_STRING) {
                        if (raw_str_used + 8 <= longest_string) {
                            memcpy(raw_str_value + raw_str_used, &buffer[data_offset], 8);
                            raw_str_used += 8;
                        }
                        offset++;
                        if (offset == col_info->width) {
                            segment_offset++;
                            if (segment_offset == var_info->n_segments) {
                                retval = readstat_convert(utf8_str_value, utf8_str_value_len, 
                                        raw_str_value, raw_str_used, ctx->converter);
                                if (retval != READSTAT_OK)
                                    goto done;
                                if (ctx->value_handler(row, var_info->index, utf8_str_value, READSTAT_TYPE_STRING, user_ctx)) {
                                    retval = READSTAT_ERROR_USER_ABORT;
                                    goto done;
                                }
                                raw_str_used = 0;
                                segment_offset = 0;
                                var_index += var_info->n_segments;
                            }
                            offset = 0;
                            col++;
                        }
                    } else if (var_info->type == READSTAT_TYPE_DOUBLE) {
                        memcpy(&fp_value, &buffer[data_offset], 8);
                        if (ctx->machine_needs_byte_swap) {
                            fp_value = byteswap_double(fp_value);
                        }
                        fp_value = handle_missing_double(fp_value, var_info);
                        if (ctx->value_handler(row, var_info->index, &fp_value, READSTAT_TYPE_DOUBLE, user_ctx)) {
                            retval = READSTAT_ERROR_USER_ABORT;
                            goto done;
                        }
                        var_index += var_info->n_segments;
                        col++;
                    }
                    data_offset += 8;
                    break;
                case 254:
                    if (var_info->type == READSTAT_TYPE_STRING) {
                        if (raw_str_used + 8 <= longest_string) {
                            memcpy(raw_str_value + raw_str_used, SAV_EIGHT_SPACES, 8);
                            raw_str_used += 8;
                        }
                        offset++;
                        if (offset == col_info->width) {
                            segment_offset++;
                            if (segment_offset == var_info->n_segments) {
                                retval = readstat_convert(utf8_str_value, utf8_str_value_len, 
                                        raw_str_value, raw_str_used, ctx->converter);
                                if (retval != READSTAT_OK)
                                    goto done;
                                if (ctx->value_handler(row, var_info->index, utf8_str_value, READSTAT_TYPE_STRING, user_ctx)) {
                                    retval = READSTAT_ERROR_USER_ABORT;
                                    goto done;
                                }
                                raw_str_used = 0;
                                segment_offset = 0;
                                var_index += var_info->n_segments;
                            }
                            offset = 0;
                            col++;
                        }
                    } else {
                        retval = READSTAT_ERROR_PARSE;
                        goto done;
                    }
                    break;
                case 255:
                    fp_value = NAN;
                    if (ctx->value_handler(row, var_info->index, &fp_value, var_info->type, user_ctx)) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        goto done;
                    }
                    var_index += var_info->n_segments;
                    col++;
                    break;
                default:
                    fp_value = value[i] - 100.0;
                    fp_value = handle_missing_double(fp_value, var_info);
                    if (ctx->value_handler(row, var_info->index, &fp_value, var_info->type, user_ctx)) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        goto done;
                    }
                    var_index += var_info->n_segments;
                    col++;
                    break;
            }
            if (col == ctx->var_index) {
                col = 0;
                var_index = 0;
                row++;
            }
        }
    }
done:
    if (retval == READSTAT_OK) {
        if (out_rows)
            *out_rows = row;
    }
    if (raw_str_value)
        free(raw_str_value);
    if (utf8_str_value)
        free(utf8_str_value);

    return retval;
}

static readstat_error_t sav_parse_machine_integer_info_record(void *data, size_t data_len, sav_ctx_t *ctx) {
    if (data_len != 32)
        return READSTAT_ERROR_PARSE;

    char *src_charset = NULL;
    sav_machine_integer_info_record_t record;
    memcpy(&record, data, data_len);
    if (ctx->machine_needs_byte_swap) {
        record.character_code = byteswap4(record.character_code);
    }
    if (record.character_code == SAV_CHARSET_7_BIT_ASCII || record.character_code == SAV_CHARSET_UTF8) {
        /* do nothing */
    } else {
        int i;
        for (i=0; i<sizeof(_charset_table)/sizeof(_charset_table[0]); i++) {
            if (record.character_code  == _charset_table[i].code) {
                src_charset = _charset_table[i].name;
                break;
            }
        }
        if (src_charset == NULL) {
            if (ctx->error_handler) {
                char error_buf[1024];
                snprintf(error_buf, sizeof(error_buf), "Unsupported character set: %d\n", record.character_code);
                ctx->error_handler(error_buf);
            }
            return READSTAT_ERROR_UNSUPPORTED_CHARSET;
        }
    }
    if (src_charset) {
        ctx->converter = iconv_open("UTF-8", src_charset);
        if (ctx->converter == (iconv_t)-1) {
            return READSTAT_ERROR_UNSUPPORTED_CHARSET;
        }
    }
    return READSTAT_OK;
}

static readstat_error_t sav_parse_machine_floating_point_record(void *data, sav_ctx_t *ctx) {
    return READSTAT_OK;
}

static readstat_error_t sav_parse_variable_display_parameter_record(void *data, sav_ctx_t *ctx) {
    return READSTAT_OK;
}

readstat_error_t readstat_parse_sav(readstat_parser_t *parser, const char *filename, void *user_ctx) {
    int fd;
    readstat_error_t retval = READSTAT_OK;
    sav_file_header_record_t header;
    sav_ctx_t *ctx = NULL;
    void *data_buf = NULL;
    size_t data_buf_capacity = 4096;
    
    if ((fd = readstat_open(filename)) == -1) {
        return READSTAT_ERROR_OPEN;
    }
    
    if ((read(fd, &header, sizeof(sav_file_header_record_t))) < sizeof(sav_file_header_record_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    ctx = sav_ctx_init(&header);
    if (ctx == NULL) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    ctx->error_handler = parser->error_handler;
    ctx->value_handler = parser->value_handler;
    ctx->value_label_handler = parser->value_label_handler;
    
    if ((data_buf = malloc(data_buf_capacity)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    while (1) {
        int32_t rec_type;
        int32_t extra_info[3];
        size_t data_len = 0;
        int i;
        int done = 0;
        if (read(fd, &rec_type, sizeof(int32_t)) < sizeof(int32_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        if (ctx->machine_needs_byte_swap) {
            rec_type = byteswap4(rec_type);
        }
        
        switch (rec_type) {
            case SAV_RECORD_TYPE_VARIABLE:
                retval = sav_read_variable_record(fd, ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_VALUE_LABEL:
                retval = sav_read_value_label_record(fd, ctx, user_ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_DOCUMENT:
                retval = sav_read_document_record(fd, ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_DICT_TERMINATION:
                retval = sav_read_dictionary_termination_record(fd, ctx);
                if (retval)
                    goto cleanup;
                done = 1;
                break;
            case SAV_RECORD_TYPE_HAS_DATA:
                if (read(fd, extra_info, sizeof(extra_info)) < sizeof(extra_info)) {
                    retval = READSTAT_ERROR_READ;
                    goto cleanup;
                }
                if (ctx->machine_needs_byte_swap) {
                    for (i=0; i<3; i++)
                        extra_info[i] = byteswap4(extra_info[i]);
                }
                int subtype = extra_info[0];
                int size = extra_info[1];
                int count = extra_info[2];
                data_len = size * count;
                if (data_buf_capacity < data_len) {
                    if ((data_buf = realloc(data_buf, data_buf_capacity = data_len)) == NULL) {
                        retval = READSTAT_ERROR_MALLOC;
                        goto cleanup;
                    }
                }
                if (read(fd, data_buf, data_len) < data_len) {
                    retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                
                switch (subtype) {
                    case SAV_RECORD_SUBTYPE_INTEGER_INFO:
                        retval = sav_parse_machine_integer_info_record(data_buf, data_len, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_FP_INFO:
                        retval = sav_parse_machine_floating_point_record(data_buf, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_VAR_DISPLAY:
                        retval = sav_parse_variable_display_parameter_record(data_buf, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_LONG_VAR_NAME:
                        retval = sav_parse_long_variable_names_record(data_buf, count, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_VERY_LONG_STR:
                        retval = sav_parse_very_long_string_record(data_buf, count, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
                        break;
                    default: /* misc. info */
                        break;
                }
                break;
            default:
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
                break;
        }
        if (done)
            break;
    }
    
    int i;
    for (i=0; i<ctx->var_index;) {
        sav_varinfo_t *info = &ctx->varinfo[i];
        if (info->string_length) {
            info->n_segments = (info->string_length + 251) / 252;
        } else {
            info->n_segments = 1;
        }
        info->index = ctx->var_count++;
        i += info->n_segments;
    }
    
    if (parser->info_handler) {
        if (parser->info_handler(ctx->record_count, ctx->var_count, user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    if (parser->variable_handler) {
        for (i=0; i<ctx->var_index;) {
            char label_name_buf[256];
            sav_varinfo_t *info = &ctx->varinfo[i];
            snprintf(label_name_buf, sizeof(label_name_buf), SAV_LABEL_NAME_PREFIX "%d", info->labels_index);

            char *format = NULL;
            char buf[80];

            if (spss_format(buf, sizeof(buf), &info->print_format)) {
                format = buf;
            }

            int cb_retval = parser->variable_handler(info->index, info->longname, format, info->label, 
                                        info->labels_index == -1 ? NULL : label_name_buf,
                                        info->type, user_ctx);
            if (cb_retval) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
            i += info->n_segments;
        }
    }
    
    if (ctx->value_handler) {
        retval = sav_read_data(fd, ctx, user_ctx);
    }
    
cleanup:
    if (fd > 0)
        readstat_close(fd);
    if (data_buf)
        free(data_buf);
    if (ctx)
        sav_ctx_free(ctx);
    
    return retval;
}

static readstat_error_t sav_emit_header(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    time_t now = time(NULL);
    struct tm *time_s = localtime(&now);

    sav_file_header_record_t header;
    memcpy(header.rec_type, "$FL2", sizeof("$FL2")-1);
    memset(header.prod_name, ' ', sizeof(header.prod_name));
    memcpy(header.prod_name,
           "@(#) SPSS DATA FILE - created by " READSTAT_PRODUCT_NAME ", " READSTAT_PRODUCT_URL, 
           sizeof("@(#) SPSS DATA FILE - created by " READSTAT_PRODUCT_NAME ", " READSTAT_PRODUCT_URL)-1);
    header.layout_code = 2;
    header.nominal_case_size = writer->row_len / 8;
    header.compressed = 0; /* TODO */
    header.weight_index = 0;
    header.ncases = writer->row_count;
    header.bias = 100.0;
    
    strftime(header.creation_date, sizeof(header.creation_date),
             "%d %b %y", time_s);
    strftime(header.creation_time, sizeof(header.creation_time),
             "%T", time_s);
    
    memset(header.file_label, ' ', sizeof(header.file_label));

    size_t file_label_len = strlen(writer->file_label);
    if (file_label_len > sizeof(header.file_label))
        file_label_len = sizeof(header.file_label);

    if (writer->file_label[0])
        memcpy(header.file_label, writer->file_label, file_label_len);
    
    memset(header.padding, '\0', sizeof(header.padding));
    
    retval = readstat_write_bytes(writer, &header, sizeof(header));
    return retval;
}

static readstat_error_t sav_emit_variable_records(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    int32_t rec_type = 0;
    double missing_val = NAN;
    
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);
        char name_data[9];
        snprintf(name_data, sizeof(name_data), "var%d", i);
        size_t name_data_len = strlen(name_data);

        const char *title_data = r_variable->label;
        size_t title_data_len = strlen(title_data);

        rec_type = SAV_RECORD_TYPE_VARIABLE;
        retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
        if (retval != READSTAT_OK)
            goto cleanup;
        
        sav_variable_record_t variable;
        memset(&variable, 0, sizeof(sav_variable_record_t));
        variable.type = (r_variable->type == READSTAT_TYPE_STRING) ? 255 : 0;
        variable.has_var_label = (title_data_len > 0);
        variable.n_missing_values = 1;
        if (r_variable->format[0]) {
            const char *fmt = r_variable->format;
            spss_format_t spss_format = { .type = 0, .width = 0, .decimal_places = 0 };
            if (spss_parse_format(fmt, strlen(fmt), &spss_format) == READSTAT_OK) {
                variable.print = (
                        (spss_format.type << 16) |
                        (spss_format.width << 8) |
                        spss_format.decimal_places);
            } else {
                retval = READSTAT_ERROR_BAD_FORMAT_STRING;
                goto cleanup;
            }
            variable.write = variable.print;
        }

        memset(variable.name, ' ', sizeof(variable.name));
        if (name_data_len > 0 && name_data_len <= sizeof(variable.name))
            memcpy(variable.name, name_data, name_data_len);
        
        retval = readstat_write_bytes(writer, &variable, sizeof(variable));
        if (retval != READSTAT_OK)
            goto cleanup;
        
        if (title_data_len > 0) {
            int32_t label_len = title_data_len;
            if (label_len > 120)
                label_len = 120;
            
            char padded_label[120];
            memcpy(padded_label, title_data, label_len);
            
            retval = readstat_write_bytes(writer, &label_len, sizeof(label_len));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, padded_label, (label_len + 3) / 4 * 4);
            if (retval != READSTAT_OK)
                goto cleanup;
        }
        
        retval = readstat_write_bytes(writer, &missing_val, sizeof(missing_val));
        if (retval != READSTAT_OK)
            goto cleanup;
        
        if (r_variable->type == READSTAT_TYPE_STRING) {
            int extra_fields = r_variable->width / 8 - 1;
            int j;
            for (j=0; j<extra_fields; j++) {
                retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
                if (retval != READSTAT_OK)
                    goto cleanup;

                memset(&variable, '\0', sizeof(variable));
                variable.type = -1;
                retval = readstat_write_bytes(writer, &variable, sizeof(variable));
                if (retval != READSTAT_OK)
                    goto cleanup;
            }
        }
    }

cleanup:
    return retval;
}

static readstat_error_t sav_emit_value_label_records(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i, j;
    for (i=0; i<writer->label_sets_count; i++) {
        readstat_label_set_t *r_label_set = readstat_get_label_set(writer, i);
        int32_t label_count = r_label_set->value_labels_count;
        int32_t rec_type = 0;
        if (label_count) {
            rec_type = SAV_RECORD_TYPE_VALUE_LABEL;

            retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, &label_count, sizeof(label_count));
            if (retval != READSTAT_OK)
                goto cleanup;
            
            readstat_types_t user_type = r_label_set->type;
            for (j=0; j<label_count; j++) {
                readstat_value_label_t *r_value_label = readstat_get_value_label(r_label_set, j);
                char value[8];
                if (user_type == READSTAT_TYPE_STRING) {
                    const char *txt_value = r_value_label->string_key;
                    size_t txt_len = strlen(txt_value);
                    if (txt_len > 8)
                        txt_len = 8;

                    memset(value, ' ', sizeof(value));
                    memcpy(value, txt_value, txt_len);
                } else if (user_type == READSTAT_TYPE_DOUBLE) {
                    double num_val = r_value_label->double_key;
                    memcpy(value, &num_val, sizeof(double));
                } else if (user_type == READSTAT_TYPE_INT32) {
                    double num_val = r_value_label->int32_key;
                    memcpy(value, &num_val, sizeof(double));
                }
                retval = readstat_write_bytes(writer, value, sizeof(value));
                
                const char *label_data = r_value_label->label;
                size_t label_data_len = strlen(label_data);
                if (label_data_len > 255)
                    label_data_len = 255;
                
                char label_len = label_data_len;
                retval = readstat_write_bytes(writer, &label_len, sizeof(label_len));
                if (retval != READSTAT_OK)
                    goto cleanup;

                char label[255];
                memset(label, ' ', sizeof(label));
                memcpy(label, label_data, label_data_len);
                retval = readstat_write_bytes(writer, label, (label_data_len + 8) / 8 * 8 - 1);
                if (retval != READSTAT_OK)
                    goto cleanup;
            }
            
            rec_type = SAV_RECORD_TYPE_VALUE_LABEL_VARIABLES;
            int32_t var_count = r_label_set->variables_count;
            
            retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, &var_count, sizeof(var_count));
            if (retval != READSTAT_OK)
                goto cleanup;

            for (j=0; j<var_count; j++) {
                readstat_variable_t *r_variable = readstat_get_label_set_variable(r_label_set, j);
                int32_t dictionary_index = 1 + r_variable->offset / 8;
                retval = readstat_write_bytes(writer, &dictionary_index, sizeof(dictionary_index));
                if (retval != READSTAT_OK)
                    goto cleanup;
            }
        }
    }

cleanup:
    return retval;
}

static readstat_error_t sav_emit_integer_info_record(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;

    sav_info_record_t info_header;
    info_header.rec_type = SAV_RECORD_TYPE_HAS_DATA;
    info_header.subtype = SAV_RECORD_SUBTYPE_INTEGER_INFO;
    info_header.size = 4;
    info_header.count = 8;
    retval = readstat_write_bytes(writer, &info_header, sizeof(info_header));
    if (retval != READSTAT_OK)
        goto cleanup;
    
    sav_machine_integer_info_record_t machine_info;
    machine_info.version_major = 1;
    machine_info.version_minor = 0;
    machine_info.version_revision = 0;
    machine_info.machine_code = -1;
    machine_info.floating_point_rep = SAV_FLOATING_POINT_REP_IEEE;
    machine_info.compression_code = 1;
    machine_info.endianness = machine_is_little_endian() ? SAV_ENDIANNESS_LITTLE : SAV_ENDIANNESS_BIG;
    machine_info.character_code = SAV_CHARSET_UTF8;
    retval = readstat_write_bytes(writer, &machine_info, sizeof(machine_info));

cleanup:
    return retval;
}

static readstat_error_t sav_emit_floating_point_info_record(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;

    sav_info_record_t info_header;
    info_header.rec_type = SAV_RECORD_TYPE_HAS_DATA;
    info_header.subtype = SAV_RECORD_SUBTYPE_FP_INFO;
    info_header.size = 8;
    info_header.count = 3;
    retval = readstat_write_bytes(writer, &info_header, sizeof(info_header));
    if (retval != READSTAT_OK)
        goto cleanup;
    
    sav_machine_floating_point_info_record_t fp_info;
    fp_info.sysmis = NAN;
    fp_info.lowest = DBL_MAX;
    fp_info.highest = -DBL_MAX;
    retval = readstat_write_bytes(writer, &fp_info, sizeof(fp_info));
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t sav_emit_long_var_name_record(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    sav_info_record_t info_header;
    info_header.rec_type = SAV_RECORD_TYPE_HAS_DATA;
    info_header.subtype = SAV_RECORD_SUBTYPE_LONG_VAR_NAME;
    info_header.size = 1;
    info_header.count = 0;
    
    for (i=0; i<writer->variables_count; i++) {
        char name_data[9];
        snprintf(name_data, sizeof(name_data), "var%d", i);
        size_t name_data_len = strlen(name_data);
        
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);
        const char *title_data = r_variable->name;
        size_t title_data_len = strlen(title_data);
        if (title_data_len > 0 && name_data_len > 0) {
            if (title_data_len > 64)
                title_data_len = 64;
            
            info_header.count += name_data_len;
            info_header.count += sizeof("=")-1;
            info_header.count += title_data_len;
            info_header.count += sizeof("\x09")-1;
        }
    }
    
    if (info_header.count > 0) {
        info_header.count--; /* no trailing 0x09 */
        
        retval = readstat_write_bytes(writer, &info_header, sizeof(info_header));
        if (retval != READSTAT_OK)
            goto cleanup;
        
        int is_first = 1;
        
        for (i=0; i<writer->variables_count; i++) {
            char name_data[9];
            snprintf(name_data, sizeof(name_data), "var%d", i);
            size_t name_data_len = strlen(name_data);

            readstat_variable_t *r_variable = readstat_get_variable(writer, i);
            const char *title_data = r_variable->name;
            size_t title_data_len = strlen(title_data);
            
            char kv_separator = '=';
            char tuple_separator = 0x09;
            
            if (title_data_len > 0) {
                if (title_data_len > 64)
                    title_data_len = 64;

                if (!is_first) {
                    retval = readstat_write_bytes(writer, &tuple_separator, sizeof(tuple_separator));
                    if (retval != READSTAT_OK)
                        goto cleanup;
                }
                
                retval = readstat_write_bytes(writer, name_data, name_data_len);
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_bytes(writer, &kv_separator, sizeof(kv_separator));
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_bytes(writer, title_data, title_data_len);
                if (retval != READSTAT_OK)
                    goto cleanup;
                
                is_first = 0;
            }
        }
    }

cleanup:
    return retval;
}

static readstat_error_t sav_emit_termination_record(readstat_writer_t *writer) {
    sav_dictionary_termination_record_t termination_record;
    termination_record.rec_type = SAV_RECORD_TYPE_DICT_TERMINATION;
    termination_record.filler = 0;
    
    return readstat_write_bytes(writer, &termination_record, sizeof(termination_record));
}

/*
static readstat_error_t sav_emit_string_value(readstat_writer_t *writer, void *user_ctx, const char *string) {
    size_t max_len = field_lengths[i];
    size_t padded_len = (max_len + 7) / 8 * 8;
    memset(text_value, ' ', padded_len);
    if (user_text_value != NULL && user_text_value[0] != '\0') {
        memcpy(text_value, user_text_value, strlen(user_text_value));
    }
    writer->data_writer(text_value, padded_len, user_ctx);
    return READSTAT_OK;
}

static readstat_error_t sav_emit_double_value(readstat_writer_t *writer, void *user_ctx, double val) {
    writer->data_writer(&val, sizeof(val), user_ctx);
    return READSTAT_OK;
}

static readstat_error_t sav_emit_observations(readstat_writer_t *writer, void *user_ctx, size_t *field_lengths) {
    readstat_error_t retval = READSTAT_OK;
    int i, j;
    char *text_value = malloc(MAX_TEXT_SIZE);
    for (j=0; j<writer->obs_count; j++) {
        for (i=0; i<writer->var_count; i++) {
            readstat_types_t user_type = writer->variable_type_provider(i, user_ctx);
            if (user_type == READSTAT_TYPE_STRING) {
                const char * user_text_value = writer->string_value_provider(j, i, user_ctx);
                retval = sav_emit_string_value(writer, user_ctx, user_text_value);
            } else {
                double val = NAN;
                if (user_type == READSTAT_TYPE_DOUBLE) {
                    val = writer->double_value_provider(j, i, user_ctx);
                } else if (user_type == READSTAT_TYPE_CHAR) {
                    val = writer->char_value_provider(j, i, user_ctx);
                } else if (user_type == READSTAT_TYPE_INT16) {
                    val = writer->int16_value_provider(j, i, user_ctx);
                } else if (user_type == READSTAT_TYPE_INT32) {
                    val = writer->int32_value_provider(j, i, user_ctx);
                } else if (user_type == READSTAT_TYPE_FLOAT) {
                    val = writer->float_value_provider(j, i, user_ctx);
                }
                retval = sav_emit_double_value(writer, user_ctx, val);
            }
            if (retval != READSTAT_OK)
                goto cleanup;
        }
    }
cleanup:
    if (text_value)
        free(text_value);

    return retval;
}

readstat_error_t readstat_write_sav(readstat_writer_t *writer, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    size_t *field_lengths = calloc(writer->var_count, sizeof(size_t));
    writer->user_ctx = user_ctx;
    for (i=0; i<writer->var_count; i++) {
        if (writer->variable_type_provider(i, user_ctx) == READSTAT_TYPE_STRING) {
            field_lengths[i] = writer->variable_width_provider(i, user_ctx);
            if (field_lengths[i] > 255)
                field_lengths[i] = 255;
        }
    }

    writer->row_len = writer->var_count;
    for (i=0; i<writer->var_count; i++) {
        if (field_lengths[i]) {
            writer->row_len += (field_lengths[i] + 7) / 8 - 1;
        }
    }
    
    retval = sav_emit_header(writer);
    if (retval != READSTAT_OK)
        goto cleanup;
    
    retval = sav_emit_variable_records(writer, user_ctx, field_lengths);
    if (retval != READSTAT_OK)
        goto cleanup;
    
    retval = sav_emit_value_label_records(writer, user_ctx, field_lengths);
    if (retval != READSTAT_OK)
        goto cleanup;
    
    retval = sav_emit_integer_info_record(writer, user_ctx);
    if (retval != READSTAT_OK)
        goto cleanup;
    
    retval = sav_emit_floating_point_info_record(writer, user_ctx);
    if (retval != READSTAT_OK)
        goto cleanup;
    
    retval = sav_emit_long_var_name_record(writer, user_ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_termination_record(writer, user_ctx);
    if (retval != READSTAT_OK)
        goto cleanup;
    
    retval = sav_emit_observations(writer, user_ctx, field_lengths);
    if (retval != READSTAT_OK)
        goto cleanup;
    
cleanup:
    if (field_lengths)
        free(field_lengths);
    
    return retval;

}
*/

static readstat_error_t sav_write_char(void *row, readstat_variable_t *var, char value) {
    if (var->type != READSTAT_TYPE_CHAR) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_int16(void *row, readstat_variable_t *var, int16_t value) {
    if (var->type != READSTAT_TYPE_INT16) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_int32(void *row, readstat_variable_t *var, int32_t value) {
    if (var->type != READSTAT_TYPE_INT32) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_float(void *row, readstat_variable_t *var, float value) {
    if (var->type != READSTAT_TYPE_FLOAT) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_double(void *row, readstat_variable_t *var, double value) {
    if (var->type != READSTAT_TYPE_DOUBLE) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_string(void *row, readstat_variable_t *var, const char *value) {
    if (var->type != READSTAT_TYPE_STRING) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    memset(row, ' ', var->width);
    if (value != NULL && value[0] != '\0') {
        memcpy(row, value, strlen(value));
    }
    return READSTAT_OK;
}

static readstat_error_t sav_write_missing(void *row, readstat_variable_t *var) {
    if (var->type == READSTAT_TYPE_STRING) {
        memset(row, ' ', var->width);
    } else {
        double dval = NAN;
        memcpy(row, &dval, sizeof(double));
    }
    return READSTAT_OK;
}

static size_t sav_variable_width(readstat_types_t type, size_t user_width) {
    if (type == READSTAT_TYPE_STRING) {
        if (user_width > 255 || user_width == 0)
            user_width = 255;
        return (user_width + 7) / 8 * 8;
    }
    return 8;
}

static readstat_error_t sav_begin_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    readstat_error_t retval = READSTAT_OK;

    retval = sav_emit_header(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_variable_records(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_value_label_records(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_integer_info_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_floating_point_info_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_long_var_name_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_termination_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

readstat_error_t readstat_begin_writing_sav(readstat_writer_t *writer, void *user_ctx,
        const char *file_label, long row_count) {
    strncpy(writer->file_label, file_label, sizeof(writer->file_label));
    writer->row_count = row_count;
    writer->user_ctx = user_ctx;

    writer->callbacks.variable_width = &sav_variable_width;
    writer->callbacks.write_char = &sav_write_char;
    writer->callbacks.write_int16 = &sav_write_int16;
    writer->callbacks.write_int32 = &sav_write_int32;
    writer->callbacks.write_float = &sav_write_float;
    writer->callbacks.write_double = &sav_write_double;
    writer->callbacks.write_string = &sav_write_string;
    writer->callbacks.write_missing = &sav_write_missing;
    writer->callbacks.begin_data = &sav_begin_data;

    return READSTAT_OK;
}
