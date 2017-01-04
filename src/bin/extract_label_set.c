#include "../readstat.h"
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "extract_label_set.h"

readstat_label_set_t * get_label_set(const char *val_labels, struct context *ctx, int alloc_new) {
    for (int i=0; i<ctx->variable_count; i++) {
        readstat_label_set_t * lbl = &ctx->label_set[i];
        if (0 == strcmp(lbl->name, val_labels)) {
            return lbl;
        }
    }
    if (!alloc_new) {
        fprintf(stderr, "%s:%d could not find value labels %s\n", __FILE__, __LINE__, val_labels);
        return NULL;
    }
    ctx->variable_count++;
    ctx->label_set = realloc(ctx->label_set, ctx->variable_count*sizeof(readstat_label_set_t));
    if (!ctx->label_set) {
        fprintf(stderr, "%s:%d realloc error: %s\n", __FILE__, __LINE__, strerror(errno));
        return NULL;
    }
    readstat_label_set_t * lbl = &ctx->label_set[ctx->variable_count-1];
    memset(lbl, 0, sizeof(readstat_label_set_t));
    snprintf(lbl->name, sizeof(lbl->name), "%s", val_labels);
    return lbl;
}


int handle_value_label(const char *val_labels, readstat_value_t value, const char *label, void *c) {
    struct context *ctx = (struct context*)c;
    if (value.type == READSTAT_TYPE_DOUBLE || value.type == READSTAT_TYPE_STRING || value.type == READSTAT_TYPE_INT32) {
        readstat_label_set_t * label_set = get_label_set(val_labels, ctx, 1);
        if (!label_set) {
            return READSTAT_ERROR_MALLOC;
        }
        long label_idx = label_set->value_labels_count;
        label_set->value_labels = realloc(label_set->value_labels, (1 + label_idx) * sizeof(readstat_value_label_t));
        if (!label_set->value_labels) {
            fprintf(stderr, "%s:%d realloc error: %s\n", __FILE__, __LINE__, strerror(errno));
            return READSTAT_ERROR_MALLOC;
        }
        readstat_value_label_t* value_label = &label_set->value_labels[label_idx];
        memset(value_label, 0, sizeof(readstat_value_label_t));
        if (value.type == READSTAT_TYPE_DOUBLE) {
            value_label->double_key = value.v.double_value;
        } else if (value.type == READSTAT_TYPE_STRING) {
            char *string_key = malloc(strlen(value.v.string_value) + 1);
            strcpy(string_key, value.v.string_value);
            value_label->string_key = string_key;
            value_label->string_key_len = strlen(value.v.string_value);
        } else if (value.type == READSTAT_TYPE_INT32) {
            value_label->int32_key = value.v.i32_value;
        } else {
            fprintf(stderr, "%s:%d unsupported type!\n", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
        char *lbl = malloc(strlen(label) + 1);
        strcpy(lbl, label);
        value_label->label = lbl;
        value_label->label_len = strlen(label);
        label_set->value_labels_count++;
    } else {
        fprintf(stderr, "%s:%d Unhandled value.type %d\n", __FILE__, __LINE__, value.type);
        exit(EXIT_FAILURE);
    }
    return READSTAT_OK;
}
