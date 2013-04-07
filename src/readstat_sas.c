

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "readstat_sas.h"

#define SAS_ALIGNMENT1_OFFSET_0  0x22
#define SAS_ALIGNMENT1_OFFSET_4  0x33

#define SAS_ALIGNMENT2_OFFSET_0  0x32
#define SAS_ALIGNMENT2_OFFSET_4  0x33

#define SAS_FILE_FORMAT_UNIX    '1'
#define SAS_FILE_FORMAT_WINDOWS '2'

#define SAS_ENDIAN_BIG       0x00
#define SAS_ENDIAN_LITTLE    0x01

#define SAS_PAGE_TYPE_META   0x0000
#define SAS_PAGE_TYPE_DATA   0x0100
#define SAS_PAGE_TYPE_MIX    0x0200
#define SAS_PAGE_TYPE_AMD    0x0400
#define SAS_PAGE_TYPE_MASK   0x0700

#define SAS_COLUMN_TYPE_NUM  0x01
#define SAS_COLUMN_TYPE_CHR  0x02

static char sas_magic_number[32] = {
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,   0xc2, 0xea, 0x81, 0x60,
    0xb3, 0x14, 0x11, 0xcf,   0xbd, 0x92, 0x08, 0x00,
    0x09, 0xc7, 0x31, 0x8c,   0x18, 0x1f, 0x10, 0x11
};

static char subheader_signature_row_size[4] = { 0xf7, 0xf7, 0xf7, 0xf7 };
static char subheader_signature_column_size[4] = { 0xf6, 0xf6, 0xf6, 0xf6 };
static char subheader_signature_counts[4] = { 0x00, 0xfc, 0xff, 0xff };
static char subheader_signature_column_text[4] = { 0xfd, 0xff, 0xff, 0xff };
static char subheader_signature_column_name[4] = { 0xff, 0xff, 0xff, 0xff };
static char subheader_signature_column_attributes[4] = { 0xfc, 0xff, 0xff, 0xff };
static char subheader_signature_column_format[4] = { 0xfe, 0xfb, 0xff, 0xff };
static char subheader_signature_column_list[4] = { 0xfe, 0xff, 0xff, 0xff };

typedef struct text_ref_s {
    int    index;
    int    offset;
    int    length;
} text_ref_t;

typedef struct col_info_s {
    text_ref_t  name_ref;
    text_ref_t  format_ref;
    text_ref_t  label_ref;

    int    offset;
    int    width;
    int    type;
} col_info_t;

typedef struct sas_ctx_s {
    readstat_handle_info_callback      info_cb;
    readstat_handle_variable_callback  variable_cb;
    readstat_handle_value_callback    value_cb;
    void         *user_ctx;
    int32_t       header_size;
    int32_t       page_size;
    int32_t       page_count;
    int           bswap;
    int           did_submit_columns;
    int32_t       row_length;
    int32_t       page_row_count;
    int32_t       parsed_row_count;
    int32_t       total_row_count;
    int32_t       column_count1;
    int32_t       column_count2;
    int32_t       column_count;
    int           text_blob_count;
    size_t       *text_blob_lengths;
    char        **text_blobs;

    int           col_names_count;
    int           col_attrs_count;
    int           col_formats_count;

    int           col_info_count;
    col_info_t   *col_info;
} sas_ctx_t;

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

static void sas_ctx_free(sas_ctx_t *ctx) {
    int i;
    if (ctx->text_blobs) {
        for (i=0; i<ctx->text_blob_count; i++) {
            free(ctx->text_blobs[i]);
        }
        free(ctx->text_blobs);
        free(ctx->text_blob_lengths);
    }
    if (ctx->col_info)
        free(ctx->col_info);

    free(ctx);
}

static readstat_errors_t sas_read_header(int fd, sas_ctx_t *ctx) {
    sas_header_start_t  header_start;
    sas_header_middle_t header_middle;
    sas_header_end_t    header_end;
    int retval = 0;
    size_t a1 = 0, a2 = 0;
    if (read(fd, &header_start, sizeof(sas_header_start_t)) < sizeof(sas_header_start_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (memcmp(header_start.magic, sas_magic_number, sizeof(sas_magic_number)) != 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (header_start.a1 == SAS_ALIGNMENT1_OFFSET_0) {
        a1 = 0;
    } else if (header_start.a1 == SAS_ALIGNMENT1_OFFSET_4) {
        a1 = 4;
    } else {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (header_start.a2 == SAS_ALIGNMENT2_OFFSET_0) {
        a2 = 0;
    } else if (header_start.a2 == SAS_ALIGNMENT2_OFFSET_4) {
        a2 = 4;
    } else {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (header_start.endian == SAS_ENDIAN_BIG) {
        ctx->bswap = machine_is_little_endian();
    } else if (header_start.endian == SAS_ENDIAN_LITTLE) {
        ctx->bswap = !machine_is_little_endian();
    } else {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (lseek(fd, 40 + a2, SEEK_CUR) == -1) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (read(fd, &header_middle, sizeof(sas_header_middle_t)) < sizeof(sas_header_middle_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    ctx->header_size = ctx->bswap ? byteswap4(header_middle.header_size) : header_middle.header_size;
    ctx->page_size = ctx->bswap ? byteswap4(header_middle.page_size) : header_middle.page_size;
    ctx->page_count = ctx->bswap ? byteswap4(header_middle.page_count) : header_middle.page_count;
    
    if (lseek(fd, 8 + a1, SEEK_CUR) == -1) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (read(fd, &header_end, sizeof(sas_header_end_t)) < sizeof(sas_header_end_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (lseek(fd, ctx->header_size, SEEK_SET) == -1) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

cleanup:
    return retval;
}

static readstat_errors_t sas_parse_row_size_subheader(char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_errors_t retval = 0;
    uint32_t total_row_count;
    uint32_t row_length, page_row_count;
    uint32_t cc1, cc2;
    memcpy(&row_length, &subheader[20], 4);
    if (ctx->bswap)
        row_length = byteswap4(row_length);
    memcpy(&total_row_count, &subheader[24], 4);
    if (ctx->bswap)
        total_row_count = byteswap4(total_row_count);

    memcpy(&cc1, &subheader[36], 4);
    if (ctx->bswap)
        cc1 = byteswap4(cc1);
    memcpy(&cc2, &subheader[40], 4);
    if (ctx->bswap)
        cc2 = byteswap4(cc2);

    memcpy(&page_row_count, &subheader[60], 4);
    if (ctx->bswap)
        page_row_count = byteswap4(page_row_count);

    ctx->row_length = row_length;
    ctx->page_row_count = page_row_count;
    ctx->total_row_count = total_row_count;
    ctx->column_count1 = cc1;
    ctx->column_count2 = cc2;
    ctx->column_count = ctx->column_count1 + ctx->column_count2;

    return retval;
}

static text_ref_t sas_parse_text_ref(char *data, sas_ctx_t *ctx) {
    text_ref_t  ref;
    uint16_t index, offset, length;

    memcpy(&index, &data[0], 2);
    if (ctx->bswap)
        index = byteswap2(index);

    memcpy(&offset, &data[2], 2);
    if (ctx->bswap)
        offset = byteswap2(offset);

    memcpy(&length, &data[4], 2);
    if (ctx->bswap)
        length = byteswap2(length);

    ref.index = index;
    ref.offset = offset;
    ref.length = length;

    return ref;
}

static readstat_errors_t copy_text_ref(char *out_buffer, text_ref_t text_ref, size_t len, sas_ctx_t *ctx) {
    if (text_ref.index < 0 || text_ref.index >= ctx->text_blob_count)
        return READSTAT_ERROR_PARSE;
    
    if (text_ref.length == 0) {
        out_buffer[0] = '\0';
        return 0;
    }

    char *blob = ctx->text_blobs[text_ref.index];

    if (text_ref.offset < 0 || text_ref.length < 0 ||
            text_ref.offset + text_ref.length > ctx->text_blob_lengths[text_ref.index])
        return READSTAT_ERROR_PARSE;

    size_t out_len = len-1;
    if (text_ref.length < out_len)
        out_len = text_ref.length;

    memcpy(out_buffer, &blob[text_ref.offset], out_len);
    out_buffer[out_len] = '\0';

    return 0;
}

static readstat_errors_t sas_parse_column_name_subheader(char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_errors_t retval = 0;
    int cmax = (len-12-8)/8;
    int i;
    char *cnp = &subheader[12];
    ctx->col_names_count += cmax;
    if (ctx->col_info_count < ctx->col_names_count) {
        ctx->col_info_count = ctx->col_names_count;
        ctx->col_info = realloc(ctx->col_info, ctx->col_info_count * sizeof(col_info_t));
    }
    for (i=ctx->col_names_count-cmax; i<ctx->col_names_count; i++) {
        ctx->col_info[i].name_ref = sas_parse_text_ref(cnp, ctx);
        cnp += 8;
    }

    return retval;
}

static readstat_errors_t sas_parse_column_attributes_subheader(char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_errors_t retval = 0;
    int cmax = (len-12-8)/12;
    int i;
    char *cap = &subheader[12];
    ctx->col_attrs_count += cmax;
    if (ctx->col_info_count < ctx->col_attrs_count) {
        ctx->col_info_count = ctx->col_attrs_count;
        ctx->col_info = realloc(ctx->col_info, ctx->col_info_count * sizeof(col_info_t));
    }
    for (i=ctx->col_attrs_count-cmax; i<ctx->col_attrs_count; i++) {
        uint32_t offset, width;
        memcpy(&offset, &cap[0], 4);
        if (ctx->bswap)
            offset = byteswap4(width);

        memcpy(&width, &cap[4], 4);
        if (ctx->bswap)
            width = byteswap4(width);

        ctx->col_info[i].offset = offset;
        ctx->col_info[i].width = width;
        if (cap[10] == SAS_COLUMN_TYPE_NUM) {
            ctx->col_info[i].type = READSTAT_TYPE_DOUBLE;
        } else if (cap[10] == SAS_COLUMN_TYPE_CHR) {
            ctx->col_info[i].type = READSTAT_TYPE_STRING;
        } else {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
        cap += 12;
    }

cleanup:

    return retval;
}

static readstat_errors_t sas_parse_column_format_subheader(char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_errors_t retval = 0;

    ctx->col_formats_count++;
    if (ctx->col_info_count < ctx->col_formats_count) {
        ctx->col_info_count = ctx->col_formats_count;
    }

    ctx->col_info[ctx->col_formats_count-1].format_ref = sas_parse_text_ref(&subheader[34], ctx);
    ctx->col_info[ctx->col_formats_count-1].label_ref = sas_parse_text_ref(&subheader[40], ctx);

    return retval;
}

static readstat_errors_t sas_parse_subheader(char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_errors_t retval = 0;

    if (len < 4) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if (memcmp(subheader, subheader_signature_row_size, 4) == 0) {
        retval = sas_parse_row_size_subheader(subheader, len, ctx);
    } else if (memcmp(subheader, subheader_signature_column_size, 4) == 0) {
    } else if (memcmp(subheader, subheader_signature_counts, 4) == 0) {
    } else if (memcmp(subheader, subheader_signature_column_text, 4) == 0) {
        ctx->text_blob_count++;
        ctx->text_blobs = realloc(ctx->text_blobs, ctx->text_blob_count * sizeof(char *));
        ctx->text_blob_lengths = realloc(ctx->text_blob_lengths,
                                         ctx->text_blob_count * sizeof(ctx->text_blob_lengths[0]));
        /* TODO handle compression */
        char *blob = malloc(len-4);
        memcpy(blob, subheader+4, len-4);
        ctx->text_blob_lengths[ctx->text_blob_count-1] = len-4;
        ctx->text_blobs[ctx->text_blob_count-1] = blob;
    } else if (memcmp(subheader, subheader_signature_column_name, 4) == 0) {
        retval = sas_parse_column_name_subheader(subheader, len, ctx);
    } else if (memcmp(subheader, subheader_signature_column_attributes, 4) == 0) {
        retval = sas_parse_column_attributes_subheader(subheader, len, ctx);
    } else if (memcmp(subheader, subheader_signature_column_format, 4) == 0) {
        retval = sas_parse_column_format_subheader(subheader, len, ctx);
    } else if (memcmp(subheader, subheader_signature_column_list, 4) == 0) {
    } else {
        retval = READSTAT_ERROR_PARSE;
    }

cleanup:

    return retval;
}

static readstat_errors_t sas_parse_rows(char *data, sas_ctx_t *ctx) {
    readstat_errors_t retval = 0;

    int i, j;
    size_t row_offset=0;
    char  string_buf[1024];
    for (i=0; i<ctx->page_row_count; i++) {
        for (j=0; j<ctx->column_count; j++) {
            int cb_retval = 0;
            col_info_t *col_info = &ctx->col_info[j];
            char *col_data = &data[row_offset+col_info->offset];
            if (col_info->type == READSTAT_TYPE_STRING) {
                memcpy(string_buf, col_data, col_info->width);
                unpad(string_buf, col_info->width);
                cb_retval = ctx->value_cb(ctx->parsed_row_count, j, string_buf,
                        READSTAT_TYPE_STRING, ctx->user_ctx);
            } else if (col_info->type == READSTAT_TYPE_DOUBLE) {
                uint64_t  val = 0;
                memcpy(&val + 8 - col_info->width, col_data, col_info->width);
                if (ctx->bswap)
                    val = byteswap8(val);

                cb_retval = ctx->value_cb(ctx->parsed_row_count, j, (double *)&val,
                        READSTAT_TYPE_DOUBLE, ctx->user_ctx);
            }
            if (cb_retval) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }
        ctx->parsed_row_count++;
        row_offset += ctx->row_length;
    }

cleanup:

    return retval;
}

static readstat_errors_t sas_parse_page(char *page, sas_ctx_t *ctx) {
    uint16_t page_type;

    readstat_errors_t retval = 0;

    memcpy(&page_type, &page[16], 2);

    if (ctx->bswap)
        page_type = byteswap2(page_type);

    char *data = NULL;

    if ((page_type & SAS_PAGE_TYPE_MASK) == SAS_PAGE_TYPE_DATA) {
        uint32_t row_count;
        memcpy(&row_count, &page[18], 4);
        if (ctx->bswap)
            row_count = byteswap4(row_count);

        ctx->page_row_count = row_count;
        data = &page[24];
    } else { 
        uint16_t subheader_count;
        memcpy(&subheader_count, &page[20], 2);
        if (ctx->bswap)
            subheader_count = byteswap2(subheader_count);

        int i;
        char *shp = &page[24];
        for (i=0; i<subheader_count; i++) {
            uint32_t offset, len;

            memcpy(&offset, &shp[0], 4);
            if (ctx->bswap)
                offset = byteswap4(offset);

            memcpy(&len, &shp[4], 4);
            if (ctx->bswap)
                len = byteswap4(len);

            if (len > 0) {
                if (offset > ctx->page_size || offset + len > ctx->page_size) {
                    retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }

                if ((retval = sas_parse_subheader(page + offset, len, ctx)) != 0) {
                    goto cleanup;
                }
            }

            shp += 12;
        }

        if ((page_type & SAS_PAGE_TYPE_MASK) == SAS_PAGE_TYPE_MIX &&
            1 /* TODO - need a byte alignment flag or something? */) {
            data = shp + ((shp-page)%8);
        }
    }
    if (data) {
        if (!ctx->did_submit_columns) {
            if (ctx->info_cb) {
                if (ctx->info_cb(ctx->total_row_count, ctx->column_count, ctx->user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }
            if (ctx->variable_cb) {
                int i;
                char name_buf[1024];
                char format_buf[1024];
                char label_buf[1024];
                for (i=0; i<ctx->column_count; i++) {
                    if ((retval = copy_text_ref(name_buf, ctx->col_info[i].name_ref, sizeof(name_buf), ctx)) != 0) {
                        goto cleanup;
                    }
                    if ((retval = copy_text_ref(format_buf, ctx->col_info[i].format_ref, sizeof(format_buf), ctx)) != 0) {
                        goto cleanup;
                    }
                    if ((retval = copy_text_ref(label_buf, ctx->col_info[i].label_ref, sizeof(label_buf), ctx)) != 0) {
                        goto cleanup;
                    }
                    int cb_retval = ctx->variable_cb(i, name_buf, format_buf, label_buf, NULL, ctx->col_info[i].type, 
                            ctx->col_info[i].type == READSTAT_TYPE_DOUBLE ? 8 : ctx->col_info[i].width, 
                            ctx->user_ctx);
                    if (cb_retval) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        goto cleanup;
                    }
                }
            }
            ctx->did_submit_columns = 1;
        }
        if (ctx->value_cb) {
            retval = sas_parse_rows(data, ctx);
        }
    } 

cleanup:

    return retval;
}

int parse_sas(const char *filename, void *user_ctx,
        readstat_handle_info_callback info_cb, 
        readstat_handle_variable_callback variable_cb,
        readstat_handle_value_callback value_cb) {
    int fd;
    readstat_errors_t retval = 0;

    sas_ctx_t  *ctx = calloc(1, sizeof(sas_ctx_t));

    ctx->info_cb = info_cb;
    ctx->variable_cb = variable_cb;
    ctx->value_cb = value_cb;
    ctx->user_ctx = user_ctx;

    if ((fd = open(filename, O_RDONLY)) == -1) {
        return READSTAT_ERROR_OPEN;
    }

    if ((retval = sas_read_header(fd, ctx)) != 0) {
        goto cleanup;
    }

    int i;
    char *page = malloc(ctx->page_size);
    for (i=0; i<ctx->page_count; i++) {
        if (read(fd, page, ctx->page_size) < ctx->page_size) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        if ((retval = sas_parse_page(page, ctx)) != 0) {
            goto cleanup;
        }
    }
    free(page);

    if (ctx->parsed_row_count != ctx->total_row_count) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    char test;
    if (read(fd, &test, 1) == 1) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

cleanup:
    if (ctx)
        sas_ctx_free(ctx);

    return retval;
}
