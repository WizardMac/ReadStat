#include <fcntl.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "readstat_dta.h"

static inline readstat_types_t dta_type_info(unsigned char typecode, size_t *max_len, dta_ctx_t *ctx);
static int dta_read_descriptors(int fd, dta_ctx_t *ctx);
static int dta_skip_expansion_fields(int fd, dta_ctx_t *ctx);


dta_ctx_t *dta_ctx_init(int16_t nvar, int32_t nobs, unsigned char byteorder, unsigned char ds_format) {
    dta_ctx_t *ctx;
    if ((ctx = malloc(sizeof(dta_ctx_t))) == NULL) {
        return NULL;
    }
    memset(ctx, 0, sizeof(dta_ctx_t));

    int machine_byteorder = DTA_HILO;
    if (machine_is_little_endian()) {
        machine_byteorder = DTA_LOHI;
    }

    if (byteorder != machine_byteorder) {
        ctx->machine_needs_byte_swap = 1;
    }

    ctx->nvar = ctx->machine_needs_byte_swap ? byteswap2(nvar) : nvar;
    ctx->nobs = ctx->machine_needs_byte_swap ? byteswap4(nobs) : nobs;
    
    ctx->machine_is_twos_complement = READSTAT_MACHINE_IS_TWOS_COMPLEMENT;

    if (ds_format < 105) {
        ctx->fmtlist_entry_len = 7;
    } else if (ds_format < 114) {
        ctx->fmtlist_entry_len = 12;
    } else {
        ctx->fmtlist_entry_len = 49;
    }
    
    if (ds_format < 111) {
        ctx->typlist_is_char = 1;
    } else {
        ctx->typlist_is_char = 0;
    }

    if (ds_format < 105) {
        ctx->expansion_len_len = 0;
    } else if (ds_format < 110) {
        ctx->expansion_len_len = 2;
    } else {
        ctx->expansion_len_len = 4;
    }
    
    if (ds_format < 110) {
        ctx->lbllist_entry_len = 9;
        ctx->variable_name_len = 9;
    } else {
        ctx->lbllist_entry_len = 33;
        ctx->variable_name_len = 33;
    }

    if (ds_format < 108) {
        ctx->variable_labels_entry_len = 32;
        ctx->data_label_len = 32;
    } else {
        ctx->variable_labels_entry_len = 81;
        ctx->data_label_len = 81;
    }

    if (ds_format < 105) {
        ctx->time_stamp_len = 0;
        ctx->value_label_table_len_len = 2;
    } else {
        ctx->time_stamp_len = 18;
        ctx->value_label_table_len_len = 4;
    }

    ctx->typlist_len = ctx->nvar * sizeof(unsigned char);
    ctx->varlist_len = ctx->variable_name_len * ctx->nvar * sizeof(char);
    ctx->srtlist_len = (ctx->nvar + 1) * sizeof(int16_t);
    ctx->fmtlist_len = ctx->fmtlist_entry_len * ctx->nvar * sizeof(char);
    ctx->lbllist_len = ctx->lbllist_entry_len * ctx->nvar * sizeof(char);
    ctx->variable_labels_len = ctx->variable_labels_entry_len * ctx->nvar * sizeof(char);

    if ((ctx->typlist = malloc(ctx->typlist_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }
    if ((ctx->varlist = malloc(ctx->varlist_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }
    if ((ctx->srtlist = malloc(ctx->srtlist_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }
    if ((ctx->fmtlist = malloc(ctx->fmtlist_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }
    if ((ctx->lbllist = malloc(ctx->lbllist_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }

    if ((ctx->variable_labels = malloc(ctx->variable_labels_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }
    
    return ctx;
}

void dta_ctx_free(dta_ctx_t *ctx) {
    if (ctx->typlist)
        free(ctx->typlist);
    if (ctx->varlist)
        free(ctx->varlist);
    if (ctx->srtlist)
        free(ctx->srtlist);
    if (ctx->fmtlist)
        free(ctx->fmtlist);
    if (ctx->lbllist)
        free(ctx->lbllist);
    if (ctx->variable_labels)
        free(ctx->variable_labels);
    free(ctx);
}

static inline readstat_types_t dta_type_info(unsigned char typecode, size_t *max_len, dta_ctx_t *ctx) {
    if (ctx->typlist_is_char) {
        if (typecode == DTA_OLD_TYPE_CODE_CHAR) {
            *max_len = 1;
            return READSTAT_TYPE_CHAR;
        } else if (typecode == DTA_OLD_TYPE_CODE_INT16) {
            *max_len = 2;
            return READSTAT_TYPE_INT16;
        } else if (typecode == DTA_OLD_TYPE_CODE_INT32) {
            *max_len = 4;
            return READSTAT_TYPE_INT32;
        } else if (typecode == DTA_OLD_TYPE_CODE_FLOAT) {
            *max_len = 4;
            return READSTAT_TYPE_FLOAT;
        } else if (typecode == DTA_OLD_TYPE_CODE_DOUBLE) {
            *max_len = 8;
            return READSTAT_TYPE_DOUBLE;
        }
        *max_len = typecode - 0x7F;
        return READSTAT_TYPE_STRING;
    }
    
    if (typecode == DTA_TYPE_CODE_CHAR) {
        *max_len = 1;
        return READSTAT_TYPE_CHAR;
    } else if (typecode == DTA_TYPE_CODE_INT16) {
        *max_len = 2;
        return READSTAT_TYPE_INT16;
    } else if (typecode == DTA_TYPE_CODE_INT32) {
        *max_len = 4;
        return READSTAT_TYPE_INT32;
    } else if (typecode == DTA_TYPE_CODE_FLOAT) {
        *max_len = 4;
        return READSTAT_TYPE_FLOAT;
    } else if (typecode == DTA_TYPE_CODE_DOUBLE) {
        *max_len = 8;
        return READSTAT_TYPE_DOUBLE;
    }
    *max_len = typecode;
    return READSTAT_TYPE_STRING;
}

static int dta_read_descriptors(int fd, dta_ctx_t *ctx) {
    if (read(fd, ctx->typlist, ctx->typlist_len) != ctx->typlist_len)
        return -1;

    if (read(fd, ctx->varlist, ctx->varlist_len) != ctx->varlist_len)
        return -1;

    if (read(fd, ctx->srtlist, ctx->srtlist_len) != ctx->srtlist_len)
        return -1;

    if (read(fd, ctx->fmtlist, ctx->fmtlist_len) != ctx->fmtlist_len)
        return -1;

    if (read(fd, ctx->lbllist, ctx->lbllist_len) != ctx->lbllist_len)
        return -1;

    if (read(fd, ctx->variable_labels, ctx->variable_labels_len) != ctx->variable_labels_len)
        return -1;

    return 0;
}

static int dta_skip_expansion_fields(int fd, dta_ctx_t *ctx) {
    if (ctx->expansion_len_len == 0)
        return 0;
    
    while (1) {
        size_t len;
        char data_type;
        if (ctx->expansion_len_len == 2) {
            dta_short_expansion_field_t  expansion_field;
            if (read(fd, &expansion_field, sizeof(expansion_field)) != sizeof(expansion_field))
                return -1;

            if (ctx->machine_needs_byte_swap) {
                len = byteswap2(expansion_field.len);
            } else {
                len = expansion_field.len;
            }
            
            data_type = expansion_field.data_type;
        } else {
            dta_expansion_field_t  expansion_field;
            if (read(fd, &expansion_field, sizeof(expansion_field)) != sizeof(expansion_field))
                return -1;
            
            if (ctx->machine_needs_byte_swap) {
                len = byteswap4(expansion_field.len);
            } else {
                len = expansion_field.len;
            }
            
            data_type = expansion_field.data_type;
        }

        if (data_type == 0 && len == 0)
            return 0;
        
        if (data_type != 1)
            return -1;

        if (lseek(fd, len, SEEK_CUR) == -1)
            return -1;
    }

    return -1;
}

int parse_dta(const char *filename, void *user_ctx,
              readstat_handle_info_callback info_cb, readstat_handle_variable_callback variable_cb,
              readstat_handle_value_callback value_cb, readstat_handle_value_label_callback value_label_cb) {
    int retval = 0;
    int i;
    size_t  record_len = 0;
    int fd;
    char *buf = NULL;
    dta_header_t  header;
    dta_ctx_t    *ctx;

    if ((fd = open(filename, O_RDONLY)) == -1) {
        return READSTAT_ERROR_OPEN;
    }

    if (read(fd, &header, sizeof(header)) != sizeof(header)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    if ((ctx = dta_ctx_init(header.nvar, header.nobs, header.byteorder, header.ds_format)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    if (info_cb) {
        if (info_cb(ctx->nobs, ctx->nvar, user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    
    if (lseek(fd, ctx->data_label_len, SEEK_CUR) == -1) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (ctx->time_stamp_len) {
        if (lseek(fd, ctx->time_stamp_len, SEEK_CUR) == -1) { /* time_stamp */
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
    }

    if (dta_read_descriptors(fd, ctx) != 0) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    for (i=0; i<ctx->nvar; i++) {
        size_t      max_len;
        readstat_types_t type = dta_type_info(ctx->typlist[i], &max_len, ctx);

        record_len += max_len;

        if (type == READSTAT_TYPE_STRING)
            max_len++; /* might append NULL */
        
        const char *variable_name = &ctx->varlist[ctx->variable_name_len*i];
        const char *variable_label = NULL;
        const char *value_labels = NULL;
        const char *variable_format = NULL;
        
        if (ctx->variable_labels[ctx->variable_labels_entry_len*i])
            variable_label = &ctx->variable_labels[ctx->variable_labels_entry_len*i];
        if (ctx->lbllist[ctx->lbllist_entry_len*i])
            value_labels = &ctx->lbllist[ctx->lbllist_entry_len*i];
        if (ctx->fmtlist[ctx->fmtlist_entry_len*i])
            variable_format = &ctx->fmtlist[ctx->fmtlist_entry_len*i];

        if (variable_cb) {
            if (variable_cb(i, variable_name, variable_format, variable_label, 
                        value_labels, type, max_len, user_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }
    }

    if (dta_skip_expansion_fields(fd, ctx) != 0) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (record_len == 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    char  str_buf[256];
    if ((buf = malloc(record_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    for (i=0; i<ctx->nobs; i++) {
        if (read(fd, buf, record_len) != record_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        int j;
        off_t offset = 0;
        for (j=0; j<ctx->nvar; j++) {
            size_t max_len;
            readstat_types_t type = dta_type_info(ctx->typlist[j], &max_len, ctx);
            void *value = NULL;

            if (type == READSTAT_TYPE_STRING) {
                int needs_null = 1;
                int k;
                for (k=0; k<max_len; k++) {
                    if (buf[offset+k] == '\0') {
                        needs_null = 0;
                        break;
                    }
                }
                if (needs_null) {
                    memcpy(str_buf, &buf[offset], max_len);
                    str_buf[max_len] = '\0';
                    value = str_buf;
                } else {
                    value = &buf[offset];
                }
            } else if (type == READSTAT_TYPE_CHAR) {
                char byte = buf[offset];
                if (ctx->machine_is_twos_complement) {
                    byte = ones_to_twos_complement1(byte);
                }
                value = byte <= DTA_MAX_CHAR ? &byte : NULL;
            } else if (type == READSTAT_TYPE_INT16) {
                int16_t num = *((int16_t *)&buf[offset]);
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap2(num);
                }
                if (ctx->machine_is_twos_complement) {
                    num = ones_to_twos_complement2(num);
                }
                value = num <= DTA_MAX_INT16 ? &num : NULL;
            } else if (type == READSTAT_TYPE_INT32) {
                int32_t num = *((int32_t *)&buf[offset]);
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap4(num);
                }
                if (ctx->machine_is_twos_complement) {
                    num = ones_to_twos_complement4(num);
                }
                value = num <= DTA_MAX_INT32 ? &num : NULL;
            } else if (type == READSTAT_TYPE_FLOAT) {
                float num = *((float *)&buf[offset]);
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap_float(num);
                }
                value = num <= DTA_MAX_FLOAT ? &num : NULL;
            } else if (type == READSTAT_TYPE_DOUBLE) {
                double num = *((double *)&buf[offset]);
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap_double(num);
                }
                value = num <= DTA_MAX_DOUBLE ? &num : NULL;
            }

            if (value_cb) {
                if (value_cb(i, j, value, type, user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }

            offset += max_len;
        }
    }
    
    if (value_label_cb) {
        while (1) {
            size_t len = 0;
            char *table_buffer;

            if (ctx->value_label_table_len_len == 2) {
                dta_short_value_label_table_header_t table_header;
                if (read(fd, &table_header, sizeof(dta_short_value_label_table_header_t)) < 
                        sizeof(dta_short_value_label_table_header_t))
                    break;

                len = table_header.len;
            
                if (ctx->machine_needs_byte_swap)
                    len = byteswap2(table_header.len);
                
                if ((table_buffer = malloc(8 * len)) == NULL) {
                    retval = READSTAT_ERROR_MALLOC;
                    goto cleanup;
                }
                
                if (read(fd, table_buffer, 8 * len) < 8 * len) {
                    free(table_buffer);
                    break;
                }
                
                int32_t l;
                for (l=0; l<len; l++) {
                    if (value_label_cb(table_header.labname, &l, READSTAT_TYPE_INT32, table_buffer + 8 * l, user_ctx)) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        free(table_buffer);
                        goto cleanup;
                    }
                }
            } else {
                dta_value_label_table_header_t table_header;
                if (read(fd, &table_header, sizeof(dta_value_label_table_header_t)) < 
                        sizeof(dta_value_label_table_header_t))
                    break;
            
                len = table_header.len;
            
                if (ctx->machine_needs_byte_swap)
                    len = byteswap4(table_header.len);
                        
                if ((table_buffer = malloc(len)) == NULL) {
                    retval = READSTAT_ERROR_MALLOC;
                    goto cleanup;
                }
                
                if (read(fd, table_buffer, len) < len) {
                    free(table_buffer);
                    break;
                }
                
                int32_t n = *(int32_t *)table_buffer;
                int32_t txtlen = *((int32_t *)table_buffer+1);
                if (ctx->machine_needs_byte_swap) {
                    n = byteswap4(n);
                    txtlen = byteswap4(txtlen);
                }
                
                if (8*n + 8 > len || 8*n + 8 + txtlen > len || n < 0 || txtlen < 0) {
                    free(table_buffer);
                    break;
                }
                
                int32_t *off = (int32_t *)table_buffer+2;
                int32_t *val = (int32_t *)table_buffer+2+n;
                char *txt = &table_buffer[8*n+8];
                int i;
                
                if (ctx->machine_needs_byte_swap) {
                    for (i=0; i<n; i++) {
                        off[i] = byteswap4(off[i]);
                        val[i] = byteswap4(val[i]);
                    }
                }
                if (ctx->machine_is_twos_complement) {
                    for (i=0; i<n; i++) {
                        val[i] = ones_to_twos_complement4(val[i]);
                    }
                }
                
                for (i=0; i<n; i++) {
                    if (off[i] < txtlen) {
                        if (value_label_cb(table_header.labname, &val[i], READSTAT_TYPE_INT32, &txt[off[i]], user_ctx)) {
                            retval = READSTAT_ERROR_USER_ABORT;
                            free(table_buffer);
                            goto cleanup;
                        }
                    }
                }
            }
            
            free(table_buffer);
        }
    }

cleanup:
    if (fd)
        close(fd);
    if (ctx)
        dta_ctx_free(ctx);
    if (buf)
        free(buf);

    return retval;
}
