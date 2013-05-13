//
//  sav.c
//  Wizard
//
//  Created by Evan Miller on 12/16/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <iconv.h>
#include "readstat_sav.h"
#include "readstat_sav_parse.h"
#include "readstat_spss.h"

#define SAV_VARINFO_INITIAL_CAPACITY  512
#define SAV_EIGHT_SPACES              "        "
#define SAV_LABEL_NAME_PREFIX         "labels"

void sav_ctx_free(sav_ctx_t *ctx);
int sav_read_variable_record(int fd, sav_ctx_t *ctx);

static void unpad(char *string, size_t len) {
    string[len] = '\0';
    /* remove space padding */
    size_t i;
    for (i=len-1; i>0; i--) {
        if (string[i] == ' ') {
            string[i] = '\0';
        } else {
            break;
        }
    }
}
                                
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

void sav_ctx_free(sav_ctx_t *ctx) {
    if (ctx->varinfo) {
        int i;
        for (i=0; i<ctx->var_count; i++) {
            free(ctx->varinfo[i].label);
        }
        free(ctx->varinfo);
    }
    free(ctx);
}

int sav_read_variable_record(int fd, sav_ctx_t *ctx) {
    sav_variable_record_t variable;
    int retval = 0;
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
    memcpy(info->name, variable.name, 8);
    unpad(info->name, 8);
    memcpy(info->longname, variable.name, 8);
    unpad(info->longname, 8);

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
        if ((info->label = malloc(label_capacity + 1)) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        if (read(fd, info->label, label_capacity) < label_capacity) {
            retval = READSTAT_ERROR_READ;
            free(info->label);
            info->label = NULL;
            goto cleanup;
        }
        unpad(info->label, label_len);
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

int sav_read_value_label_record(int fd, sav_ctx_t *ctx, readstat_handle_value_label_callback value_label_cb, void *user_ctx) {
    int32_t label_count;
    int retval = 0;
    int32_t *vars = NULL;
    int32_t rec_type;
    int32_t var_count;
    readstat_types_t value_type = READSTAT_TYPE_STRING;
    char label_name_buf[256];
    typedef struct value_label_s {
        char          value[8];
        unsigned char label_len;
        char          label[256];
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
    
    sprintf(label_name_buf, SAV_LABEL_NAME_PREFIX "%d", ctx->value_labels_count);
    int i;
    for (i=0; i<label_count; i++) {
        value_label_t *vlabel = &value_labels[i];
        if (read(fd, vlabel, 9) < 9) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        size_t label_len = (vlabel->label_len + 8) / 8 * 8 - 1;
        if (read(fd, vlabel->label, label_len) < label_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        unpad(vlabel->label, vlabel->label_len);
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
        sav_varinfo_t *var = bsearch_b(&var_offset, ctx->varinfo, ctx->var_index, sizeof(sav_varinfo_t), ^(const void *elem1, const void *elem2) {
            int offset = *(int *)elem1;
            const sav_varinfo_t *v = (const sav_varinfo_t *)elem2;
            if (offset < v->offset)
                return -1;
            return (offset > v->offset);
        });
        if (var) {
            value_type = var->type;
            var->labels_index = ctx->value_labels_count;
        }
    }
    for (i=0; i<label_count; i++) {
        value_label_t *vlabel = &value_labels[i];
        if (value_type == READSTAT_TYPE_DOUBLE) {
            double val_d = *(double *)vlabel->value;
            if (ctx->machine_needs_byte_swap)
                val_d = byteswap_double(val_d);
            value_label_cb(label_name_buf, &val_d, value_type, vlabel->label, user_ctx);
        } else {
            char unpadded_val[9];
            memcpy(unpadded_val, vlabel->value, 8);
            unpad(unpadded_val, 8);
            value_label_cb(label_name_buf, unpadded_val, value_type, vlabel->label, user_ctx);
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

int sav_read_document_record(int fd, sav_ctx_t *ctx) {
    int32_t n_lines;
    int retval = 0;
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

int sav_read_dictionary_termination_record(int fd, sav_ctx_t *ctx) {
    int32_t filler;
    int retval = 0;
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
    
    return fp_value;
}

readstat_errors_t sav_read_data(int fd, sav_ctx_t *ctx, readstat_handle_value_callback value_cb, void *user_ctx) {
    unsigned char value[8];
    char *str_value = NULL;
    unsigned char uncompressed_value[8];
    int offset = 0;
    int segment_offset = 0;
    int row = 0, var_index = 0, col = 0;
    readstat_errors_t retval = 0;
    int i;
    double fp_value;
    int longest_string = 256;
    for (i=0; i<ctx->var_count; i++) {
        sav_varinfo_t *info = &ctx->varinfo[i];
        if (info->string_length > longest_string) {
            longest_string = info->string_length;
        }
    }
    if ((str_value = malloc(longest_string + 1)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto done;
    }
    while (1) {
        if (read(fd, value, 8) < 8) {
            break;
        }
        
        sav_varinfo_t *col_info = &ctx->varinfo[col];
        sav_varinfo_t *var_info = &ctx->varinfo[var_index];
        if (ctx->data_is_compressed) {
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
                        if (read(fd, uncompressed_value, 8) < 8) {
                            retval = READSTAT_ERROR_READ;
                            goto done;
                        }
                        if (var_info->type == READSTAT_TYPE_STRING) {
                            memcpy(str_value + segment_offset * 255 + offset * 8, uncompressed_value, 8);
                            offset++;
                            if (offset == col_info->width) {
                                segment_offset++;
                                if (segment_offset == var_info->n_segments) {
                                    unpad(str_value, (segment_offset-1) * 255 + offset * 8);
                                    if (value_cb(row, var_info->index, str_value, READSTAT_TYPE_STRING, user_ctx)) {
                                        retval = READSTAT_ERROR_USER_ABORT;
                                        goto done;
                                    }
                                    segment_offset = 0;
                                    var_index += var_info->n_segments;
                                }
                                offset = 0;
                                col++;
                            }
                        } else if (var_info->type == READSTAT_TYPE_DOUBLE) {
                            fp_value = *(double *)uncompressed_value;
                            if (ctx->machine_needs_byte_swap) {
                                fp_value = byteswap_double(fp_value);
                            }
                            fp_value = handle_missing_double(fp_value, var_info);
                            if (value_cb(row, var_info->index, &fp_value, READSTAT_TYPE_DOUBLE, user_ctx)) {
                                retval = READSTAT_ERROR_USER_ABORT;
                                goto done;
                            }
                            var_index += var_info->n_segments;
                            col++;
                        }
                        break;
                    case 254:
                        if (var_info->type == READSTAT_TYPE_STRING) {
                            memcpy(str_value + segment_offset * 255 + offset * 8, SAV_EIGHT_SPACES, 8);
                            offset++;
                            if (offset == col_info->width) {
                                segment_offset++;
                                if (segment_offset == var_info->n_segments) {
                                    unpad(str_value, (segment_offset-1) * 255 + offset * 8);
                                    if (value_cb(row, var_info->index, str_value, READSTAT_TYPE_STRING, user_ctx)) {
                                        retval = READSTAT_ERROR_USER_ABORT;
                                        goto done;
                                    }
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
                        if (value_cb(row, var_info->index, &fp_value, var_info->type, user_ctx)) {
                            retval = READSTAT_ERROR_USER_ABORT;
                            goto done;
                        }
                        var_index += var_info->n_segments;
                        col++;
                        break;
                    default:
                        fp_value = value[i] - 100.0;
                        fp_value = handle_missing_double(fp_value, var_info);
                        if (value_cb(row, var_info->index, &fp_value, var_info->type, user_ctx)) {
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
        } else {
            if (offset > 31) {
                retval = READSTAT_ERROR_PARSE;
                goto done;
            }
            if (var_info->type == READSTAT_TYPE_STRING) {
                memcpy(str_value + segment_offset * 255 + offset * 8, value, 8);
                offset++;
                if (offset == col_info->width) {
                    segment_offset++;
                    if (segment_offset == var_info->n_segments) {
                        unpad(str_value, (segment_offset-1) * 255 + offset * 8);
                        if (value_cb(row, var_info->index, str_value, READSTAT_TYPE_STRING, user_ctx)) {
                            retval = READSTAT_ERROR_USER_ABORT;
                            goto done;
                        }
                        segment_offset = 0;
                        var_index += var_info->n_segments;
                    }
                    offset = 0;
                    col++;
                }
            } else if (var_info->type == READSTAT_TYPE_DOUBLE) {
                fp_value = *(double *)value;
                if (ctx->machine_needs_byte_swap) {
                    fp_value = byteswap_double(fp_value);
                }
                fp_value = handle_missing_double(fp_value, var_info);
                if (value_cb(row, var_info->index, &fp_value, READSTAT_TYPE_DOUBLE, user_ctx)) {
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
        }
    }
    if (row != ctx->record_count) {
        retval = READSTAT_ERROR_PARSE;
    }
done:
    
    if (str_value)
        free(str_value);
    
    return retval;
}

int sav_parse_machine_integer_info_record(void *data, sav_ctx_t *ctx) {
    return 0;
}

int sav_parse_machine_floating_point_record(void *data, sav_ctx_t *ctx) {
    return 0;
}

int sav_parse_variable_display_parameter_record(void *data, sav_ctx_t *ctx) {
    return 0;
}

int parse_sav(const char *filename, void *user_ctx,
              readstat_handle_info_callback info_cb, readstat_handle_variable_callback variable_cb,
              readstat_handle_value_callback value_cb, readstat_handle_value_label_callback value_label_cb) {
    int fd;
    readstat_errors_t retval = 0;
    sav_file_header_record_t header;
    sav_ctx_t *ctx = NULL;
    void *data_buf = NULL;
    size_t data_buf_capacity = 4096;
    iconv_t converter = (iconv_t)-1;
    
    if ((fd = open(filename, O_RDONLY)) == -1) {
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
                retval = sav_read_value_label_record(fd, ctx, value_label_cb, user_ctx);
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
                        retval = sav_parse_machine_integer_info_record(data_buf, ctx);
                        if (retval)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_FP_INFO:
                        retval = sav_parse_machine_floating_point_record(data_buf, ctx);
                        if (retval)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_VAR_DISPLAY:
                        retval = sav_parse_variable_display_parameter_record(data_buf, ctx);
                        if (retval)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_LONG_VAR_NAME:
                        retval = sav_parse_long_variable_names_record(data_buf, count, ctx);
                        if (retval)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_VERY_LONG_STR:
                        retval = sav_parse_very_long_string_record(data_buf, count, ctx);
                        if (retval)
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
    
    if (info_cb) {
        if (info_cb(ctx->record_count, ctx->var_count, user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    if (variable_cb) {
        converter = iconv_open("UTF-8", "WINDOWS-1252");
        if (converter == (iconv_t)-1) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }

        for (i=0; i<ctx->var_index;) {
            char label_name_buf[256];
            sav_varinfo_t *info = &ctx->varinfo[i];
            sprintf(label_name_buf, SAV_LABEL_NAME_PREFIX "%d", info->labels_index);

            char *format = NULL;
            char buf[80];
            if (spss_format_is_date(info->print_format.type)) {
                const char *fmt = spss_format(info->print_format.type);
                sprintf(buf, "%%ts%s", fmt ? fmt : "");
                format = buf;
            }

            char varname_buf[256];

            char *inbuf = info->longname;
            size_t inbytesleft = strlen(info->longname) + 1;
            char *outbuf = varname_buf;
            size_t outbytesleft = sizeof(varname_buf);
            if (iconv(converter, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t)-1) {
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }

            int cb_retval = variable_cb(info->index, varname_buf, format, info->label, 
                                        info->labels_index == -1 ? NULL : label_name_buf,
                                        info->type, 0, user_ctx);
            if (cb_retval) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
            i += info->n_segments;
        }
    }
    
    if (value_cb) {
        retval = sav_read_data(fd, ctx, value_cb, user_ctx);
    }
    
cleanup:
    if (converter != (iconv_t)-1)
        iconv_close(converter);
    if (fd > 0)
        close(fd);
    if (data_buf)
        free(data_buf);
    if (ctx)
        sav_ctx_free(ctx);
    
    return retval;
}
