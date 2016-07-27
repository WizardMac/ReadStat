
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iconv.h>

#include "readstat.h"
#include "readstat_writer.h"
#include "readstat_sas.h"

#define COLUMN_TEXT_SIZE 1024
#define HEADER_SIZE 1024
#define PAGE_SIZE   4096

typedef struct sas_subheader_s {
    uint32_t    signature;
    char       *data;
    size_t      len;
} sas_subheader_t;

typedef struct sas_subheader_array_s {
    int64_t             count;
    sas_subheader_t   **subheaders;
} sas_subheader_array_t;

typedef struct sas_column_text_s {
    char       *data;
    size_t      used;
    int64_t     index;
} sas_column_text_t;

typedef struct sas_column_text_array_s {
    int64_t               count;
    sas_column_text_t   **column_texts;
} sas_column_text_array_t;

static int32_t sas_count_meta_pages(sas_header_info_t *hinfo, sas_subheader_array_t *sarray) {
    int i;
    int pages = 1;
    size_t bytes_left = PAGE_SIZE - (hinfo->u64 ? 40 : 24);
    size_t shp_ptr_size = hinfo->u64 ? 24 : 12;
    for (i=sarray->count-1; i>=0; i--) {
        sas_subheader_t *subheader = sarray->subheaders[i];
        if (subheader->len + shp_ptr_size > bytes_left) {
            bytes_left = PAGE_SIZE - (hinfo->u64 ? 40 : 24);
            pages++;
        }
        bytes_left -= (subheader->len + shp_ptr_size);
    }
    return pages;
}

static int32_t sas_count_data_pages(readstat_writer_t *writer) {
    return 0;
}

static sas_column_text_t *sas_column_text_init(int64_t index) {
    sas_column_text_t *column_text = calloc(1, sizeof(sas_column_text_t));
    column_text->data = malloc(COLUMN_TEXT_SIZE);
    column_text->index = index;
    return column_text;
}

static void sas_column_text_free(sas_column_text_t *column_text) {
    free(column_text->data);
    free(column_text);
}

static void sas_column_text_array_free(sas_column_text_array_t *column_text_array) {
    int i;
    for (i=0; i<column_text_array->count; i++) {
        sas_column_text_free(column_text_array->column_texts[i]);
    }
    free(column_text_array->column_texts);
    free(column_text_array);
}

static sas_text_ref_t make_text_ref(sas_column_text_array_t *column_text_array,
        const char *string) {
    size_t len = strlen(string);
    size_t padded_len = (len + 3) / 4 * 4;
    sas_column_text_t *column_text = column_text_array->column_texts[
        column_text_array->count-1];
    if (column_text->used + padded_len > COLUMN_TEXT_SIZE) {
        column_text_array->count++;
        column_text_array->column_texts = realloc(column_text_array->column_texts,
                sizeof(sas_column_text_t *) * column_text_array->count);

        column_text = sas_column_text_init(column_text_array->count-1);
        column_text_array->column_texts[column_text_array->count-1] = column_text;
    }
    sas_text_ref_t text_ref = {
        .index = column_text->index,
        .offset = column_text->used + 28,
        .length = len
    };
    strncpy(&column_text->data[column_text->used], string, padded_len);
    column_text->used += padded_len;
    return text_ref;
}

static sas_header_info_t *sas_header_info_init(readstat_writer_t *writer) {
    sas_header_info_t *hinfo = calloc(1, sizeof(sas_header_info_t));
    hinfo->creation_time = writer->timestamp;
    hinfo->modification_time = writer->timestamp;
    hinfo->header_size = HEADER_SIZE;
    hinfo->page_size = PAGE_SIZE;

    return hinfo;
}

static readstat_error_t sas_emit_header(readstat_writer_t *writer, sas_header_info_t *hinfo) {
    readstat_error_t retval = READSTAT_OK;
    struct tm epoch_tm = { .tm_year = 60, .tm_mday = 1 };
    time_t epoch = mktime(&epoch_tm);

    sas_header_start_t header_start = {
        .a2 = SAS_ALIGNMENT_OFFSET_0,
        .a1 = SAS_ALIGNMENT_OFFSET_0,
        .endian = machine_is_little_endian() ? SAS_ENDIAN_LITTLE : SAS_ENDIAN_BIG,
        .file_format = SAS_FILE_FORMAT_UNIX,
        .encoding = 20, /* UTF-8 */
        .file_type = "SAS FILE",
        .file_info = "DATA ~ ~"
    };

    memcpy(&header_start.magic, sas7bdat_magic_number, sizeof(header_start.magic));
    strncpy(header_start.file_label, writer->file_label, sizeof(header_start.file_label));

    sas_header_end_t header_end = {
        .release = "9.0101M3",
        .host = "W32_VSPRO"
    };

    retval = readstat_write_bytes(writer, &header_start, sizeof(sas_header_start_t));
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = readstat_write_zeros(writer, hinfo->pad1);
    if (retval != READSTAT_OK)
        goto cleanup;

    double creation_time = hinfo->creation_time - epoch;

    retval = readstat_write_bytes(writer, &creation_time, sizeof(double));
    if (retval != READSTAT_OK)
        goto cleanup;

    double modification_time = hinfo->modification_time - epoch;

    retval = readstat_write_bytes(writer, &modification_time, sizeof(double));
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = readstat_write_zeros(writer, 16);
    if (retval != READSTAT_OK)
        goto cleanup;

    uint32_t header_size = hinfo->header_size;
    uint32_t page_size = hinfo->page_size;

    retval = readstat_write_bytes(writer, &header_size, sizeof(uint32_t));
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = readstat_write_bytes(writer, &page_size, sizeof(uint32_t));
    if (retval != READSTAT_OK)
        goto cleanup;

    if (hinfo->u64) {
        uint64_t page_count = hinfo->page_count;
        retval = readstat_write_bytes(writer, &page_count, sizeof(uint64_t));
    } else {
        uint32_t page_count = hinfo->page_count;
        retval = readstat_write_bytes(writer, &page_count, sizeof(uint32_t));
    }
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = readstat_write_zeros(writer, 8);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = readstat_write_bytes(writer, &header_end, sizeof(sas_header_end_t));
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = readstat_write_zeros(writer, hinfo->header_size-writer->bytes_written);
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static sas_subheader_t *sas_subheader_init(uint32_t signature, size_t len) {
    sas_subheader_t *subheader = calloc(1, sizeof(sas_subheader_t));
    subheader->signature = signature;
    subheader->len = len;
    subheader->data = calloc(1, len);

    return subheader;
}

static sas_subheader_t *sas_row_size_subheader_init(readstat_writer_t *writer, 
        sas_header_info_t *hinfo) {
    sas_subheader_t *subheader = sas_subheader_init(
            SAS_SUBHEADER_SIGNATURE_ROW_SIZE,
            hinfo->u64 ? 128 : 64);

    if (hinfo->u64) {
        int64_t row_length = 0;
        int64_t row_count = writer->row_count;
        int64_t page_size = hinfo->page_size;

        memcpy(&subheader->data[40], &row_length, sizeof(int64_t));
        memcpy(&subheader->data[48], &row_count, sizeof(int64_t));
        memcpy(&subheader->data[104], &page_size, sizeof(int64_t));
        // memset(&subheader->data[128], 0xFF, 16);
    } else {
        int32_t row_length = 0;
        int32_t row_count = writer->row_count;
        int32_t page_size = hinfo->page_size;

        memcpy(&subheader->data[20], &row_length, sizeof(int32_t));
        memcpy(&subheader->data[24], &row_count, sizeof(int32_t));
        memcpy(&subheader->data[52], &page_size, sizeof(int32_t));
        // memset(&subheader->data[64], 0xFF, 8);
    }

    return subheader;
}

static sas_subheader_t *sas_col_size_subheader_init(readstat_writer_t *writer, 
        sas_header_info_t *hinfo) {
    sas_subheader_t *subheader = sas_subheader_init(
            SAS_SUBHEADER_SIGNATURE_COLUMN_SIZE,
            hinfo->u64 ? 24 : 12);
    if (hinfo->u64) {
        int64_t col_count = writer->variables_count;
        memcpy(&subheader->data[8], &col_count, sizeof(int64_t));
    } else {
        int32_t col_count = writer->variables_count;
        memcpy(&subheader->data[4], &col_count, sizeof(int32_t));
    }
    return subheader;
}

static sas_subheader_t *sas_col_name_subheader_init(readstat_writer_t *writer,
        sas_header_info_t *hinfo, sas_column_text_array_t *column_text_array) {
    sas_subheader_t *subheader = sas_subheader_init(
            SAS_SUBHEADER_SIGNATURE_COLUMN_NAME,
            hinfo->u64 ? 28+8*writer->variables_count :
            20+8*writer->variables_count);
    int i;
    char *ptrs = hinfo->u64 ? &subheader->data[16] : &subheader->data[12];
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *variable = readstat_get_variable(writer, i);
        const char *name = readstat_variable_get_name(variable);
        sas_text_ref_t text_ref = make_text_ref(column_text_array, name);
        memcpy(&ptrs[0], &text_ref.index, sizeof(uint16_t));
        memcpy(&ptrs[2], &text_ref.offset, sizeof(uint16_t));
        memcpy(&ptrs[4], &text_ref.length, sizeof(uint16_t));

        ptrs += 8;
    }
    return subheader;
}

static sas_subheader_t *sas_col_attrs_subheader_init(readstat_writer_t *writer,
        sas_header_info_t *hinfo) {
    sas_subheader_t *subheader = sas_subheader_init(
            SAS_SUBHEADER_SIGNATURE_COLUMN_ATTRS,
            hinfo->u64 ? 28+16*writer->variables_count :
            20+16*writer->variables_count);
    char *ptrs = hinfo->u64 ? &subheader->data[16] : &subheader->data[12];
    uint64_t offset = 0;
    int i;
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *variable = readstat_get_variable(writer, i);
        const char *name = readstat_variable_get_name(variable);
        readstat_type_t type = readstat_variable_get_type(variable);
        uint16_t name_length_flag = strlen(name) <= 8 ? 4 : 2048;
        uint32_t width = 8;
        if (hinfo->u64) {
            memcpy(&ptrs[0], &offset, sizeof(uint64_t));
            ptrs += sizeof(uint64_t);
        } else {
            uint32_t offset32 = offset;
            memcpy(&ptrs[0], &offset32, sizeof(uint32_t));
            ptrs += sizeof(uint32_t);
        }
        if (type == READSTAT_TYPE_STRING ||
                type == READSTAT_TYPE_LONG_STRING) {
            ptrs[6] = SAS_COLUMN_TYPE_CHR;
            width = readstat_variable_get_storage_width(variable);
        } else {
            ptrs[6] = SAS_COLUMN_TYPE_NUM;
        }
        memcpy(&ptrs[0], &width, sizeof(uint32_t));
        memcpy(&ptrs[4], &name_length_flag, sizeof(uint16_t));
        offset += width;
        ptrs += 8;
    }
    return subheader;
}

static sas_subheader_t *sas_col_format_subheader_init(readstat_variable_t *variable,
        sas_header_info_t *hinfo, sas_column_text_array_t *column_text_array) {
    sas_subheader_t *subheader = sas_subheader_init(
            SAS_SUBHEADER_SIGNATURE_COLUMN_FORMAT,
            hinfo->u64 ? 64 : 52);
    const char *format = readstat_variable_get_format(variable);
    const char *label = readstat_variable_get_label(variable);
    off_t format_offset = hinfo->u64 ? 46 : 34;
    off_t label_offset = hinfo->u64 ? 52 : 40;
    if (format) {
        sas_text_ref_t text_ref = make_text_ref(column_text_array, format);
        memcpy(&subheader->data[format_offset+0], &text_ref.index, sizeof(uint16_t));
        memcpy(&subheader->data[format_offset+2], &text_ref.offset, sizeof(uint16_t));
        memcpy(&subheader->data[format_offset+4], &text_ref.length, sizeof(uint16_t));
    }
    if (label) {
        sas_text_ref_t text_ref = make_text_ref(column_text_array, label);
        memcpy(&subheader->data[label_offset+0], &text_ref.index, sizeof(uint16_t));
        memcpy(&subheader->data[label_offset+2], &text_ref.offset, sizeof(uint16_t));
        memcpy(&subheader->data[label_offset+4], &text_ref.length, sizeof(uint16_t));
    }
    return subheader;
}

static sas_subheader_t *sas_col_text_subheader_init(readstat_writer_t *writer,
        sas_header_info_t *hinfo, sas_column_text_t *column_text) {
    sas_subheader_t *subheader = sas_subheader_init(
            SAS_SUBHEADER_SIGNATURE_COLUMN_TEXT,
            hinfo->u64 ? 36 + column_text->used
            : 32 + column_text->used);
    uint16_t used = column_text->used;
    if (hinfo->u64) {
        memcpy(&subheader->data[8], &used, sizeof(uint16_t));
        memset(&subheader->data[20], ' ', 8);
        memcpy(&subheader->data[36], column_text->data, column_text->used);
    } else {
        memcpy(&subheader->data[4], &used, sizeof(uint16_t));
        memset(&subheader->data[16], ' ', 8);
        memcpy(&subheader->data[32], column_text->data, column_text->used);
    }
    return subheader;
}

static sas_subheader_array_t *sas_subheader_array_init(readstat_writer_t *writer,
        sas_header_info_t *hinfo) {
    sas_subheader_t *row_size_subheader = NULL;
    sas_subheader_t *col_size_subheader = NULL;
    sas_subheader_t *col_name_subheader = NULL;
    sas_subheader_t *col_attrs_subheader = NULL;

    sas_column_text_array_t *column_text_array = calloc(1, sizeof(sas_column_text_array_t));
    column_text_array->count = 1;
    column_text_array->column_texts = malloc(sizeof(sas_column_text_t *));
    column_text_array->column_texts[0] = sas_column_text_init(0);

    row_size_subheader = sas_row_size_subheader_init(writer, hinfo);
    col_size_subheader = sas_col_size_subheader_init(writer, hinfo);
    col_name_subheader = sas_col_name_subheader_init(writer, hinfo, column_text_array);
    col_attrs_subheader = sas_col_attrs_subheader_init(writer, hinfo);

    long subheader_count = 4+writer->variables_count;

    sas_subheader_array_t *sarray = calloc(1, sizeof(sas_subheader_array_t));
    sarray->subheaders = calloc(subheader_count, sizeof(sas_subheader_t *));
    sarray->count = subheader_count;

    long idx = 0;

    sarray->subheaders[idx++] = row_size_subheader;
    sarray->subheaders[idx++] = col_size_subheader;
    sarray->subheaders[idx++] = col_name_subheader;
    sarray->subheaders[idx++] = col_attrs_subheader;

    int i;
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *variable = readstat_get_variable(writer, i);
        sarray->subheaders[idx++] = sas_col_format_subheader_init(variable, hinfo, column_text_array);
    }
    subheader_count += column_text_array->count;
    sarray->subheaders = realloc(sarray->subheaders, subheader_count * sizeof(sas_subheader_t *));
    for (i=column_text_array->count-1; i>=0; i--) {
        sarray->subheaders[idx++] = sas_col_text_subheader_init(writer, hinfo, 
                column_text_array->column_texts[i]);
    }
    sas_column_text_array_free(column_text_array);

    return sarray;
}

static void sas_subheader_free(sas_subheader_t *subheader) {
    if (!subheader)
        return;
    if (subheader->data)
        free(subheader->data);
    free(subheader);
}

static void sas_subheader_array_free(sas_subheader_array_t *sarray) {
    int i;
    for (i=0; i<sarray->count; i++) {
        sas_subheader_free(sarray->subheaders[i]);
    }
    free(sarray->subheaders);
    free(sarray);
}

static int sas_subheader_type(uint32_t signature) {
    return (signature == SAS_SUBHEADER_SIGNATURE_COLUMN_TEXT ||
            signature == SAS_SUBHEADER_SIGNATURE_COLUMN_NAME ||
            signature == SAS_SUBHEADER_SIGNATURE_COLUMN_ATTRS ||
            signature == SAS_SUBHEADER_SIGNATURE_COLUMN_LIST);
}

static readstat_error_t sas_emit_meta_pages(readstat_writer_t *writer, sas_header_info_t *hinfo,
        sas_subheader_array_t *sarray) {
    readstat_error_t retval = READSTAT_OK;
    int16_t page_type = SAS_PAGE_TYPE_META;
    char *page = malloc(PAGE_SIZE);

    while (sarray->count) {
        memset(page, 0, PAGE_SIZE);
        int16_t shp_count = 0;
        size_t shp_data_offset = PAGE_SIZE;
        size_t shp_ptr_offset = 0;
        size_t shp_ptr_size = 0;
        if (hinfo->u64) {
            memcpy(&page[32], &page_type, sizeof(int16_t));
            shp_ptr_size = 24;
            shp_ptr_offset = 40;
        } else {
            memcpy(&page[16], &page_type, sizeof(int16_t));
            shp_ptr_size = 12;
            shp_ptr_offset = 24;
        }

        while (sarray->count && 
                sarray->subheaders[sarray->count-1]->len + shp_ptr_size <
                shp_data_offset - shp_ptr_offset) {
            sas_subheader_t *subheader = sarray->subheaders[sarray->count-1];

            /* copy ptr */
            if (hinfo->u64) {
                uint64_t offset = shp_data_offset - subheader->len;
                uint64_t len = subheader->len;
                memcpy(&page[shp_ptr_offset], &offset, sizeof(uint64_t));
                memcpy(&page[shp_ptr_offset+8], &len, sizeof(uint64_t));
                page[shp_ptr_offset+17] = sas_subheader_type(subheader->signature);
            } else {
                uint32_t offset = shp_data_offset - subheader->len;
                uint32_t len = subheader->len;
                memcpy(&page[shp_ptr_offset], &offset, sizeof(uint32_t));
                memcpy(&page[shp_ptr_offset+4], &len, sizeof(uint32_t));
                page[shp_ptr_offset+9] = sas_subheader_type(subheader->signature);
            }
            shp_ptr_offset += shp_ptr_size;

            /* copy data */
            shp_data_offset -= subheader->len;
            memcpy(&page[shp_data_offset], subheader->data, subheader->len);

            sas_subheader_free(subheader);
            sarray->count--;
            shp_count++;
        }
        if (hinfo->u64) {
            memcpy(&page[34], &shp_count, sizeof(int16_t));
            memcpy(&page[36], &shp_count, sizeof(int16_t));
        } else {
            memcpy(&page[18], &shp_count, sizeof(int16_t));
            memcpy(&page[20], &shp_count, sizeof(int16_t));
        }

        retval = readstat_write_bytes(writer, page, PAGE_SIZE);
        if (retval != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    free(page);

    return retval;
}

static readstat_error_t sas_begin_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    readstat_error_t retval = READSTAT_OK;
    sas_header_info_t *hinfo = sas_header_info_init(writer);
    sas_subheader_array_t *sarray = sas_subheader_array_init(writer, hinfo);

    hinfo->page_count = sas_count_meta_pages(hinfo, sarray) + sas_count_data_pages(writer);

    retval = sas_emit_header(writer, hinfo);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sas_emit_meta_pages(writer, hinfo, sarray);
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    sas_subheader_array_free(sarray);
    if (retval != READSTAT_OK) {
        free(hinfo);
    } else {
        writer->module_ctx = hinfo;
    }

    return retval;
}

static readstat_error_t sas_end_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    sas_header_info_t *hinfo = (sas_header_info_t *)writer->module_ctx;
    free(hinfo);
    return READSTAT_OK;
}

readstat_error_t readstat_begin_writing_sas7bdat(readstat_writer_t *writer, void *user_ctx, long row_count) {
    writer->row_count = row_count;
    writer->user_ctx = user_ctx;

    writer->callbacks.begin_data = &sas_begin_data;
    writer->callbacks.end_data = &sas_end_data;

    writer->initialized = 1;

    return READSTAT_OK;
}
