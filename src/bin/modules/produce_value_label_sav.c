#include <stdio.h>
#include <stdlib.h>

#include "../../readstat.h"
#include "../format.h"
#include "json_metadata.h"

#include "produce_csv_column_header.h"
#include "../util/readstat_sav_date.h"

void produce_value_label_double_date_sav(const char* column, struct csv_metadata *c, const char *code, const char *label) {
    char *endptr;
    double v = readstat_sav_date_parse(code, &endptr);
    if (endptr == code) {
        fprintf(stderr, "%s:%d not a valid date: %s\n", __FILE__, __LINE__, code);
        exit(EXIT_FAILURE);
    }
    readstat_value_t value = {
        .v = { .double_value = v },
        .type = READSTAT_TYPE_DOUBLE,
    };
    c->parser->value_label_handler(column, value, label, c->user_ctx);
}

void produce_value_label_string(const char* column, struct csv_metadata *c, const char *code, const char *label) {
    readstat_value_t value = {
        .v = { .string_value = code },
        .type = READSTAT_TYPE_STRING,
    };
    c->parser->value_label_handler(column, value, label, c->user_ctx);
}

void produce_value_label_double_sav(const char* column, struct csv_metadata *c, const char *code, const char *label) {
    char *endptr;
    double v = strtod(code, &endptr);
    if (endptr == code) {
        fprintf(stderr, "%s:%d not a number: %s\n", __FILE__, __LINE__, code);
        exit(EXIT_FAILURE);
    }
    readstat_value_t value = {
        .v = { .double_value = v },
        .type = READSTAT_TYPE_DOUBLE,
    };
    c->parser->value_label_handler(column, value, label, c->user_ctx);
}

void produce_value_label_sav(struct csv_metadata *c, const char* column) {
    readstat_variable_t* variable = &c->variables[c->columns];
    readstat_type_t coltype = variable->type;
    jsmntok_t* categories = find_variable_property(c->json_md->js, c->json_md->tok, column, "categories");
    if (categories==NULL) {
        return;
    }
    int is_date = c->is_date[c->columns];
    int j = 1;
    char code_buf[1024];
    char label_buf[1024];
    for (int i=0; i<categories->size; i++) {
        jsmntok_t* tok = categories+j;
        char* code = get_object_property(c->json_md->js, tok, "code", code_buf, sizeof(code_buf));
        char* label = get_object_property(c->json_md->js, tok, "label", label_buf, sizeof(label_buf));
        if (!code || !label) {
            fprintf(stderr, "%s:%d bogus JSON metadata input. Missing code/label for column %s\n", __FILE__, __LINE__, column);
            exit(EXIT_FAILURE);
        }
        if (is_date) {
            produce_value_label_double_date_sav(column, c, code, label);
        } else if (coltype == READSTAT_TYPE_DOUBLE) {
            produce_value_label_double_sav(column, c, code, label);
        } else if (coltype == READSTAT_TYPE_STRING) {
            produce_value_label_string(column, c, code, label);
        } else {
            fprintf(stderr, "%s:%d unsupported column type %d for value label %s\n", __FILE__, __LINE__, coltype, column);
            exit(EXIT_FAILURE);
        }
        j += slurp_object(tok);
    }
}
