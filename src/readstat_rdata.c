//
//  readstat_rdata.c
//  Wizard
//
//  Created by Evan Miller on 3/31/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include "readstat_rdata.h"

#define RDATA_ATOM_LEN 128

typedef struct rdata_atom_table_s {
    int   count;
    char *data;
} rdata_atom_table_t;

typedef struct rdata_ctx_s {
    int                                  machine_needs_byteswap;
    readstat_handle_table_callback       handle_table;
    readstat_handle_column_callback      handle_column;
    readstat_handle_column_name_callback handle_column_name;
    readstat_handle_text_value_callback  handle_text_value;
    readstat_handle_text_value_callback  handle_value_label;
    void                                *user_ctx;
    int                                  fd;
    
    rdata_atom_table_t                  *atom_table;
} rdata_ctx_t;

static int atom_table_add(rdata_atom_table_t *table, char *key);
static char *atom_table_lookup(rdata_atom_table_t *table, int index);

static int read_environment(char *table_name, rdata_ctx_t *ctx);
static int read_sexptype_header(rdata_sexptype_info_t *header, rdata_ctx_t *ctx);
static int read_length(int32_t *outLength, rdata_ctx_t *ctx);
static int read_string_vector(int32_t length, readstat_handle_text_value_callback handle_text, rdata_ctx_t *ctx);
static int read_logical_vector(char *name, rdata_ctx_t *ctx);
static int read_integer_vector(char *name, rdata_ctx_t *ctx);
static int read_real_vector(char *name, rdata_ctx_t *ctx);
static int read_value_vector(rdata_sexptype_header_t header, char *name, rdata_ctx_t *ctx);
static int read_character_string(char *key, size_t keylen, rdata_ctx_t *ctx);
static int read_generic_list(int attributes, rdata_ctx_t *ctx);
static int read_attributes(int (*handle_attribute)(char *key, rdata_sexptype_info_t val_info, rdata_ctx_t *ctx),
                           rdata_ctx_t *ctx);
static int recursive_discard(rdata_sexptype_header_t sexptype_header, rdata_ctx_t *ctx);

static int atom_table_add(rdata_atom_table_t *table, char *key) {
    table->data = realloc(table->data, RDATA_ATOM_LEN * (table->count + 1));
    memcpy(&table->data[RDATA_ATOM_LEN*table->count], key, strlen(key) + 1);
    table->count++;
    return table->count;
}

static char *atom_table_lookup(rdata_atom_table_t *table, int index) {
    return &table->data[(index-1)*RDATA_ATOM_LEN];
}

int parse_rdata(const char *filename, void *user_ctx,
                readstat_handle_table_callback handle_table,
                readstat_handle_column_callback handle_column,
                readstat_handle_column_name_callback handle_column_name,
                readstat_handle_text_value_callback handle_text_value,
                readstat_handle_text_value_callback handle_value_label) {
    int retval = 1;
    rdata_ctx_t *ctx = malloc(sizeof(rdata_ctx_t));
    rdata_atom_table_t *atom_table = malloc(sizeof(rdata_atom_table_t));
    
    atom_table->count = 0;
    atom_table->data = NULL;
    
    ctx->user_ctx = user_ctx;
    ctx->handle_table = handle_table;
    ctx->handle_column = handle_column;
    ctx->handle_column_name = handle_column_name;
    ctx->handle_text_value = handle_text_value;
    ctx->handle_value_label = handle_value_label;
    ctx->atom_table = atom_table;
    
    int test_byte_order = 1;
    ctx->machine_needs_byteswap = 0;
    if (((char *)&test_byte_order)[0]) {
        ctx->machine_needs_byteswap = 1;
    }
    
    if ((ctx->fd = open(filename, O_RDONLY)) == -1) {
        return READSTAT_ERROR_OPEN;
    }
    
    char header[5];
    
    rdata_v2_header_t v2_header;
    
    if (read(ctx->fd, &header, sizeof(header)) != sizeof(header)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (strncmp("RDX2\n", header, sizeof(header)) != 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
    if (read(ctx->fd, &v2_header, sizeof(v2_header)) != sizeof(v2_header)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (ctx->machine_needs_byteswap) {
        v2_header.format_version = byteswap4(v2_header.format_version);
        v2_header.writer_version = byteswap4(v2_header.writer_version);
        v2_header.reader_version = byteswap4(v2_header.reader_version);
    }
    
    retval = read_environment(NULL, ctx);
    if (retval != 0)
        goto cleanup;
    
    char test;
    
    if (read(ctx->fd, &test, 1) == 1) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
cleanup:
    close(ctx->fd);
    free(ctx->atom_table);
    free(ctx);
    
    return retval;
}

static int read_environment(char *table_name, rdata_ctx_t *ctx) {
    int retval = 0;
    
    while (1) {
        rdata_sexptype_info_t sexptype_info;
        
        retval = read_sexptype_header(&sexptype_info, ctx);
        if (retval != 0)
            goto cleanup;
        
        if (sexptype_info.header.type == RDATA_PSEUDO_SXP_NIL)
            break;
        
        if (sexptype_info.header.type != RDATA_SEXPTYPE_PAIRLIST) {
            retval = recursive_discard(sexptype_info.header, ctx);
            if (retval != 0)
                goto cleanup;
            continue;
        }
        
        char *key = atom_table_lookup(ctx->atom_table, sexptype_info.ref);
        
        retval = read_sexptype_header(&sexptype_info, ctx);
        if (retval != 0)
            goto cleanup;
        
        if (sexptype_info.header.type == RDATA_SEXPTYPE_REAL_VECTOR ||
            sexptype_info.header.type == RDATA_SEXPTYPE_INTEGER_VECTOR ||
            sexptype_info.header.type == RDATA_SEXPTYPE_LOGICAL_VECTOR) {
            if (table_name == NULL && ctx->handle_table) {
                if (ctx->handle_table(key, ctx->user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }   
            }
            retval = read_value_vector(sexptype_info.header, key, ctx);
            if (retval != 0)
                goto cleanup;
        } else if (sexptype_info.header.type == RDATA_SEXPTYPE_CHARACTER_VECTOR) {
            if (table_name == NULL && ctx->handle_table) {
                if (ctx->handle_table(key, ctx->user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }   
            }
            int32_t length;
            retval = read_length(&length, ctx);
            if (retval != 0)
                goto cleanup;
            
            if (ctx->handle_column) {
                if (ctx->handle_column(key, READSTAT_TYPE_STRING, NULL, length, ctx->user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }
            retval = read_string_vector(length, ctx->handle_text_value, ctx);
            if (retval != 0)
                goto cleanup;
        } else if (sexptype_info.header.type == RDATA_SEXPTYPE_GENERIC_VECTOR &&
                   sexptype_info.header.object && sexptype_info.header.attributes) {
            if (table_name != NULL) {
                retval = recursive_discard(sexptype_info.header, ctx);
            } else {
                if (ctx->handle_table) {
                    if (ctx->handle_table(key, ctx->user_ctx)) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        goto cleanup;
                    }
                }
                retval = read_generic_list(sexptype_info.header.attributes, ctx);
            }
            if (retval != 0)
                goto cleanup;
        } else {
            retval = recursive_discard(sexptype_info.header, ctx);
            if (retval != 0)
                goto cleanup;
        }
    }
    
cleanup:
    
    return retval;
}

static int read_sexptype_header(rdata_sexptype_info_t *header_info, rdata_ctx_t *ctx) {
    uint32_t sexptype;
    rdata_sexptype_header_t header;
    int retval = 0;
    if (read(ctx->fd, &sexptype, sizeof(sexptype)) != sizeof(sexptype)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byteswap)
        sexptype = byteswap4(sexptype);
    
    memcpy(&header, &sexptype, sizeof(sexptype));
    uint32_t attributes = 0, tag = 0, ref = 0;

    if (header.type == RDATA_SEXPTYPE_PAIRLIST) {
        if (header.attributes) {
            if (read(ctx->fd, &attributes, sizeof(attributes)) != sizeof(attributes)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            if (ctx->machine_needs_byteswap)
                header_info->attributes = byteswap4(header_info->attributes);
        }
        if (header.tag) {
            if (read(ctx->fd, &tag, sizeof(tag)) != sizeof(tag)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            if (ctx->machine_needs_byteswap)
                tag = byteswap4(tag);
        }
        
        if (tag == 1) {
            rdata_sexptype_info_t key_info;
            retval = read_sexptype_header(&key_info, ctx);
            if (retval != 0)
                goto cleanup;
            
            if (key_info.header.type != RDATA_SEXPTYPE_CHARACTER_STRING) {
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }
                        
            char key[RDATA_ATOM_LEN];
            retval = read_character_string(key, RDATA_ATOM_LEN, ctx);
            if (retval != 0)
                goto cleanup;

            ref = atom_table_add(ctx->atom_table, key);
        } else if ((tag & 0xFF) == RDATA_PSEUDO_SXP_REF) {
            ref = (tag >> 8);
        }
    }
    
    header_info->header = header;
    header_info->attributes = attributes;
    header_info->tag = tag;
    header_info->ref = ref;
        
cleanup:
    
    return retval;
}

static int handle_vector_attribute(char *key, rdata_sexptype_info_t val_info, rdata_ctx_t *ctx) {
    int retval = 0;
    if (strcmp(key, "levels") == 0) {
        int32_t length;
        retval = read_length(&length, ctx);
        if (retval != 0)
            return retval;
        
        retval = read_string_vector(length, ctx->handle_value_label, ctx);
    } else {
        retval = recursive_discard(val_info.header, ctx);
    }
    return retval;
}

static int read_character_string(char *key, size_t keylen, rdata_ctx_t *ctx) {
    uint32_t length;
    int retval = 0;
    
    if (read(ctx->fd, &length, sizeof(length)) != sizeof(length)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (ctx->machine_needs_byteswap)
        length = byteswap4(length);
    
    if (length == -1) {
        key[0] = '\0';
        return 0;
    }
    
    if (length + 1 > keylen) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
    if (read(ctx->fd, key, length) != length) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    key[length] = '\0';
    
cleanup:
    
    return retval;
}

static int handle_data_frame_attribute(char *key, rdata_sexptype_info_t val_info, rdata_ctx_t *ctx) {
    int discard_value = 0;
    int retval = 0;
    
    if (strcmp(key, "names") != 0) {
        discard_value = 1;
    }
    
    if (val_info.header.type != RDATA_SEXPTYPE_CHARACTER_VECTOR)
        discard_value = 1;
    
    if (!discard_value) {
        int32_t length;
        retval = read_length(&length, ctx);
        if (retval != 0)
            return retval;
        
        retval = read_string_vector(length, ctx->handle_column_name, ctx);
    } else {
        retval = recursive_discard(val_info.header, ctx);
    }
    
    return retval;
}

static int read_attributes(int (*handle_attribute)(char *key, rdata_sexptype_info_t val_info, rdata_ctx_t *ctx),
                           rdata_ctx_t *ctx) {
    rdata_sexptype_info_t pairlist_info, val_info;
    int retval = 0;
    
    retval = read_sexptype_header(&pairlist_info, ctx);
    if (retval != 0)
        goto cleanup;
    
    while (pairlist_info.header.type == RDATA_SEXPTYPE_PAIRLIST) {
        int discard_value = 0;
        
        /* value */
        retval = read_sexptype_header(&val_info, ctx);
        if (retval != 0)
            goto cleanup;
        
        if (discard_value) {
            retval = recursive_discard(val_info.header, ctx);
            if (retval != 0)
                goto cleanup;
        } else {
            char *key = atom_table_lookup(ctx->atom_table, pairlist_info.ref);
            retval = handle_attribute(key, val_info, ctx);
            if (retval != 0)
                goto cleanup;
        }
        
        /* next */
        retval = read_sexptype_header(&pairlist_info, ctx);
        if (retval != 0)
            goto cleanup;
    }

cleanup:
    
    return retval;
}

static int read_generic_list(int attributes, rdata_ctx_t *ctx) {
    int retval = 0;
    int32_t length;
    int i;
    rdata_sexptype_info_t sexptype_info;
    
    retval = read_length(&length, ctx);
    if (retval != 0)
        goto cleanup;
    
    for (i=0; i<length; i++) {        
        retval = read_sexptype_header(&sexptype_info, ctx);
        if (retval != 0)
            goto cleanup;
        
        if (sexptype_info.header.type == RDATA_SEXPTYPE_CHARACTER_VECTOR) {
            int32_t vec_length;
            retval = read_length(&vec_length, ctx);
            if (retval != 0)
                goto cleanup;
            if (ctx->handle_column) {
                if (ctx->handle_column(NULL, READSTAT_TYPE_STRING, NULL, vec_length, ctx->user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }
            retval = read_string_vector(vec_length, ctx->handle_text_value, ctx);
        } else {
            retval = read_value_vector(sexptype_info.header, NULL, ctx);
        }
        if (retval != 0)
            goto cleanup;
    }
    
    if (attributes) {
        retval = read_attributes(&handle_data_frame_attribute, ctx);
        if (retval != 0)
            goto cleanup;
    }
    
cleanup:
    
    return retval;
}

static int read_length(int32_t *outLength, rdata_ctx_t *ctx) {
    int32_t length;
    int retval = 0;
    
    if (read(ctx->fd, &length, sizeof(length)) != sizeof(length)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (ctx->machine_needs_byteswap)
        length = byteswap4(length);
    
    if (outLength)
        *outLength = length;
    
cleanup:
    
    return retval;
}

static int read_string_vector(int32_t length, readstat_handle_text_value_callback handle_text, rdata_ctx_t *ctx) {
    int32_t string_length;
    int retval = 0;
    rdata_sexptype_info_t info;
    size_t buffer_size = 4096;
    char *buffer = NULL;
    int i;

    buffer = malloc(buffer_size);
    
    for (i=0; i<length; i++) {
        retval = read_sexptype_header(&info, ctx);
        if (retval != 0)
            goto cleanup;
        
        if (info.header.type != RDATA_SEXPTYPE_CHARACTER_STRING) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
        
        if (read(ctx->fd, &string_length, sizeof(string_length)) != sizeof(string_length)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        if (ctx->machine_needs_byteswap)
            string_length = byteswap4(string_length);
        
        if (string_length + 1 > buffer_size) {
            buffer = realloc(buffer, string_length + 1);
            if (buffer == NULL) {
                retval = READSTAT_ERROR_MALLOC;
                goto cleanup;
            }
        }
        
        if (read(ctx->fd, buffer, string_length) != string_length) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        buffer[string_length] = '\0';
        
        if (handle_text) {
            if (handle_text(buffer, i, ctx->user_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }
    }
cleanup:
    
    if (buffer)
        free(buffer);
    
    return retval;
}


static int read_value_vector(rdata_sexptype_header_t header, char *name, rdata_ctx_t *ctx) {
    int retval = 0;
    switch (header.type) {
        case RDATA_SEXPTYPE_REAL_VECTOR:
            retval = read_real_vector(name, ctx);
            break;
        case RDATA_SEXPTYPE_INTEGER_VECTOR:
            retval = read_integer_vector(name, ctx);
            break;
        case RDATA_SEXPTYPE_LOGICAL_VECTOR:
            retval = read_logical_vector(name, ctx);
            break;
        default:
            retval = READSTAT_ERROR_PARSE;
            break;
    }
    
    if (retval != 0)
        goto cleanup;
    
    if (header.attributes) {
        retval = read_attributes(&handle_vector_attribute, ctx);
        if (retval != 0)
            goto cleanup;
    }
    
cleanup:
    
    return retval;
}

static int read_logical_vector(char *name, rdata_ctx_t *ctx) {
    int32_t length;
    int retval = 0;
    
    int32_t *vals = NULL;
    float *real_vals = NULL;

    int i;
    
    retval = read_length(&length, ctx);
    if (retval != 0)
        goto cleanup;
    
    vals = malloc(length * sizeof(int32_t));
    if (vals == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    if (read(ctx->fd, vals, length * sizeof(int32_t)) != length * sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (ctx->machine_needs_byteswap) {
        for (i=0; i<length; i++) {
            vals[i] = byteswap4(vals[i]);
        }
    }
    
    real_vals = malloc(length * sizeof(float));
    for (i=0; i<length; i++) {
        if (vals[i] == INT32_MIN) {
            real_vals[i] = NAN;
        } else {
            real_vals[i] = vals[i];
        }
    }
    
    if (ctx->handle_column) {
        if (ctx->handle_column(name, READSTAT_TYPE_FLOAT, vals, length, ctx->user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    
cleanup:
    if (vals)
        free(vals);
    if (real_vals)
        free(real_vals);
    
    return retval;
}

static int read_integer_vector(char *name, rdata_ctx_t *ctx) {
    int32_t length;
    int retval = 0;
    
    int32_t *vals = NULL;
    int i;
    
    retval = read_length(&length, ctx);
    if (retval != 0)
        goto cleanup;
    
    vals = malloc(length * sizeof(int32_t));
    if (vals == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    if (read(ctx->fd, vals, length * sizeof(int32_t)) != length * sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (ctx->machine_needs_byteswap) {
        for (i=0; i<length; i++) {
            vals[i] = byteswap4(vals[i]);
        }
    }
    
    if (ctx->handle_column) {
        if (ctx->handle_column(name, READSTAT_TYPE_INT32, vals, length, ctx->user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    
cleanup:
    if (vals)
        free(vals);
    
    return retval;
}

static int read_real_vector(char *name, rdata_ctx_t *ctx) {
    int32_t length;
    int retval = 0;
    
    double *vals = NULL;
    int i;
    
    retval = read_length(&length, ctx);
    if (retval != 0)
        goto cleanup;
    
    vals = malloc(length * sizeof(double));
    if (vals == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    if (read(ctx->fd, vals, length * sizeof(double)) != length * sizeof(double)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (ctx->machine_needs_byteswap) {
        for (i=0; i<length; i++) {
            vals[i] = byteswap_double(vals[i]);
        }
    }
    
    if (ctx->handle_column) {
        if (ctx->handle_column(name, READSTAT_TYPE_DOUBLE, vals, length, ctx->user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }

    
cleanup:
    if (vals)
        free(vals);
    
    return retval;
}

static int discard_vector(rdata_sexptype_header_t sexptype_header, size_t element_size, rdata_ctx_t *ctx) {
    int32_t length;
    int retval = 0;
    
    retval = read_length(&length, ctx);
    if (retval != 0)
        goto cleanup;
    
    if (length > 0) {
        if (lseek(ctx->fd, length * element_size, SEEK_CUR) == -1) {
            return READSTAT_ERROR_READ;
        }
    } else {
        printf("Vector with non-positive length: %d\n", length);
    }
    
    if (sexptype_header.attributes) {
        rdata_sexptype_info_t temp_info;
        retval = read_sexptype_header(&temp_info, ctx);
        if (retval != 0)
            goto cleanup;
        
        retval = recursive_discard(temp_info.header, ctx);
    }
    
cleanup:
    
    return retval;
}

static int discard_character_string(int add_to_table, rdata_ctx_t *ctx) {
    int retval = 0;
    char key[128];
    
    retval = read_character_string(key, RDATA_ATOM_LEN, ctx);
    if (retval != 0)
        goto cleanup;
    
    if (strlen(key) == 0) {
        printf("String with non-positive length: %ld\n", strlen(key));
        
        /*
         rdata_sexptype_info_t temp_info;
         retval = read_sexptype_header(&temp_info, ctx);
         if (retval != 0)
         goto cleanup;
         
         if (temp_info.header.type != RDATA_PSEUDO_SXP_NIL) {
         retval = READSTAT_ERROR_PARSE;
         goto cleanup;
         }
         */
    } else if (add_to_table) {
        atom_table_add(ctx->atom_table, key);
    }
    
cleanup:
    
    return retval;
}

static int discard_pairlist(rdata_sexptype_header_t sexptype_header, rdata_ctx_t *ctx) {
    rdata_sexptype_info_t temp_info;
    int error = 0;
    while (1) {
        switch (sexptype_header.type) {
            case RDATA_SEXPTYPE_PAIRLIST:                
                /* value */
                error = read_sexptype_header(&temp_info, ctx);
                if (error != 0)
                    return error;
                error = recursive_discard(temp_info.header, ctx);
                if (error != 0)
                    return error;
                
                /* tail */
                error = read_sexptype_header(&temp_info, ctx);
                if (error != 0)
                    return error;
                sexptype_header = temp_info.header;
                break;
            case RDATA_PSEUDO_SXP_NIL:
                goto done;
            default:
                return READSTAT_ERROR_PARSE;
        }
    }
done:
    
    return 0;
}

static int recursive_discard(rdata_sexptype_header_t sexptype_header, rdata_ctx_t *ctx) {
    uint32_t length;
    rdata_sexptype_info_t info;
    rdata_sexptype_info_t prot, tag;
    
    int error = 0;
    int i;
    
    switch (sexptype_header.type) {
        case RDATA_SEXPTYPE_SYMBOL:
            error = read_sexptype_header(&info, ctx);
            if (error != 0)
                goto cleanup;
            
            error = recursive_discard(info.header, ctx);
            if (error != 0)
                goto cleanup;
            break;
        case RDATA_PSEUDO_SXP_PERSIST:
        case RDATA_PSEUDO_SXP_NAMESPACE:
        case RDATA_PSEUDO_SXP_PACKAGE:
            error = read_sexptype_header(&info, ctx);
            if (error != 0)
                goto cleanup;
            
            error = recursive_discard(info.header, ctx);
            if (error != 0)
                goto cleanup;
            break;
        case RDATA_SEXPTYPE_BUILTIN_FUNCTION:
        case RDATA_SEXPTYPE_SPECIAL_FUNCTION:
            error = discard_character_string(0, ctx);
            break;
        case RDATA_SEXPTYPE_PAIRLIST:
            error = discard_pairlist(sexptype_header, ctx);
            break;
        case RDATA_SEXPTYPE_CHARACTER_STRING:
            error = discard_character_string(1, ctx);
            break;
        case RDATA_SEXPTYPE_RAW_VECTOR:
            error = discard_vector(sexptype_header, 1, ctx);
            break;
        case RDATA_SEXPTYPE_LOGICAL_VECTOR:
            error = discard_vector(sexptype_header, 4, ctx);
            break;
        case RDATA_SEXPTYPE_INTEGER_VECTOR:
            error = discard_vector(sexptype_header, 4, ctx);
            break;
        case RDATA_SEXPTYPE_REAL_VECTOR:
            error = discard_vector(sexptype_header, 8, ctx);
            break;
        case RDATA_SEXPTYPE_COMPLEX_VECTOR:
            error = discard_vector(sexptype_header, 16, ctx);
            break;
        case RDATA_SEXPTYPE_CHARACTER_VECTOR:
            if (read(ctx->fd, &length, sizeof(length)) != sizeof(length)) {
                return READSTAT_ERROR_READ;
            }
            if (ctx->machine_needs_byteswap)
                length = byteswap4(length);
            
            for (i=0; i<length; i++) {
                error = read_sexptype_header(&info, ctx);
                if (error != 0)
                    goto cleanup;
                if (info.header.type != RDATA_SEXPTYPE_CHARACTER_STRING) {
                    error = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                
                error = discard_character_string(0, ctx);
                if (error != 0)
                    goto cleanup;
            }
            break;
        case RDATA_SEXPTYPE_GENERIC_VECTOR:
        case RDATA_SEXPTYPE_EXPRESSION_VECTOR:
            if (read(ctx->fd, &length, sizeof(length)) != sizeof(length)) {
                return READSTAT_ERROR_READ;
            }
            if (ctx->machine_needs_byteswap)
                length = byteswap4(length);
            
            for (i=0; i<length; i++) {
                error = read_sexptype_header(&info, ctx);
                if (error != 0)
                    goto cleanup;
                error = recursive_discard(info.header, ctx);
                if (error != 0)
                    goto cleanup;
            }
            break;
        case RDATA_SEXPTYPE_DOT_DOT_DOT:
        case RDATA_SEXPTYPE_PROMISE:
        case RDATA_SEXPTYPE_LANGUAGE_OBJECT: 
        case RDATA_SEXPTYPE_CLOSURE:
            if (sexptype_header.attributes) {
                error = read_sexptype_header(&info, ctx);
                if (error != 0)
                    goto cleanup;
                
                error = recursive_discard(info.header, ctx);
                if (error != 0)
                    goto cleanup;
            }
            if (sexptype_header.tag) {
                error = read_sexptype_header(&info, ctx);
                if (error != 0)
                    goto cleanup;
                
                error = recursive_discard(info.header, ctx);
                if (error != 0)
                    goto cleanup;
            }
            /* CAR */
            error = read_sexptype_header(&info, ctx);
            if (error != 0)
                goto cleanup;
            
            error = recursive_discard(info.header, ctx);
            if (error != 0)
                goto cleanup;
            
            /* CDR */
            error = read_sexptype_header(&info, ctx);
            if (error != 0)
                goto cleanup;
            
            error = recursive_discard(info.header, ctx);
            if (error != 0)
                goto cleanup;
            break;
        case RDATA_SEXPTYPE_EXTERNAL_POINTER:
            read_sexptype_header(&prot, ctx);
            recursive_discard(prot.header, ctx);
            
            read_sexptype_header(&tag, ctx);
            recursive_discard(tag.header, ctx);
            break;
        case RDATA_SEXPTYPE_ENVIRONMENT:
            /* locked */
            if (lseek(ctx->fd, sizeof(uint32_t), SEEK_CUR) == -1) {
                return READSTAT_ERROR_READ;
            }
            
            rdata_sexptype_info_t enclosure, frame, hash_table, attributes;
            read_sexptype_header(&enclosure, ctx);
            recursive_discard(enclosure.header, ctx);
            
            read_sexptype_header(&frame, ctx);
            recursive_discard(frame.header, ctx);
            
            read_sexptype_header(&hash_table, ctx);
            recursive_discard(hash_table.header, ctx);
            
            read_sexptype_header(&attributes, ctx);
            recursive_discard(attributes.header, ctx);
            /*
             if (sexptype_header.attributes) {
             if (lseek(ctx->fd, sizeof(uint32_t), SEEK_CUR) == -1) {
             return READSTAT_ERROR_READ;
             }
             } */
            break;
        case RDATA_PSEUDO_SXP_REF:
        case RDATA_PSEUDO_SXP_NIL:
        case RDATA_PSEUDO_SXP_GLOBAL_ENVIRONMENT:
        case RDATA_PSEUDO_SXP_UNBOUND_VALUE:
        case RDATA_PSEUDO_SXP_MISSING_ARGUMENT:
        case RDATA_PSEUDO_SXP_BASE_NAMESPACE:
        case RDATA_PSEUDO_SXP_EMPTY_ENVIRONMENT:
        case RDATA_PSEUDO_SXP_BASE_ENVIRONMENT:
            break;
        default:
            return READSTAT_ERROR_READ;
    }
cleanup:
    
    return error;
}

