#include "../readstat.h"
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "util/readstat_sav_date.h"
#include "util/readstat_dta_days.h"
#include "format.h"
#include "extract_metadata.h"
#include "extract_label_set.h"
#include "write_missing_values.h"
#include "write_value_labels.h"


int escape(const char *s, char* dest) {
    char c = s[0];
    if (c == '\\') {
        if (dest) {
            dest[0] = '\\';
            dest[1] = '\\';
        }
        return 2 + escape(&s[1], dest ? &dest[2] : NULL);
    } else if (c == '"') {
        if (dest) {
            dest[0] = '\\';
            dest[1] = '"';
        }
        return 2 + escape(&s[1], dest ? &dest[2] : NULL);
    } else if (c) {
        if (dest) {
            dest[0] = c;
        }
        return 1 + escape(&s[1], dest ? &dest[1] : NULL);
    } else {
        if (dest) {
            dest[0] = '"';
            dest[1] = 0;
        }
        return 1;
    }
}

char* quote_and_escape(const char *src) {
    int newlen = 2 + escape(src, NULL);
    char *dest = malloc(newlen);
    dest[0] = '"';
    escape(src, &dest[1]);
    return dest;
}

int extract_decimals(const char *s, char prefix) {
    if (s && s[0] && s[0]==prefix) {
        int decimals;
        if (sscanf(s, "%*c%*d.%d", &decimals) == 1) {
            if (decimals < 0 || decimals > 16) {
                fprintf(stderr, "%s:%d decimals was %d, expected to be [0, 16]\n", __FILE__, __LINE__, decimals);
                exit(EXIT_FAILURE);
            }
            return decimals;
        }
        fprintf(stderr, "%s:%d not a number: %s\n", __FILE__, __LINE__, &s[1]);
        exit(EXIT_FAILURE);
    } else {
        return -1;
    }
}

int handle_variable_sav(int index, readstat_variable_t *variable, const char *val_labels, struct context *ctx) {
    char* type = "";
    const char *format = readstat_variable_get_format(variable);
    const char *label = readstat_variable_get_label(variable);
    int sav_date = format && (strcmp(format, "EDATE40") == 0) && variable->type == READSTAT_TYPE_DOUBLE;
    int decimals = -1;

    if (variable->type == READSTAT_TYPE_STRING) {
        type = "STRING";
    } else if (sav_date) {
        type = "DATE";
    } else if (variable->type == READSTAT_TYPE_DOUBLE) {
        type = "NUMERIC";
        decimals = extract_decimals(format, 'F');
    } else {
        fprintf(stderr, "%s:%d unhandled type %s\n", __FILE__, __LINE__, readstat_type_str(variable->type));
        exit(EXIT_FAILURE);
    }

    if (ctx->count == 0) {
        ctx->count = 1;
        fprintf(ctx->fp, "{\"type\": \"SPSS\",\n  \"variables\": [\n");
    } else {
        fprintf(ctx->fp, ",\n");
    }

    fprintf(ctx->fp, "{\"type\": \"%s\", \"name\": \"%s\"", type, variable->name);
    if (decimals > 0) {
        fprintf(ctx->fp, ", \"decimals\": %d", decimals);
    }
    if (label) {
        char* quoted_label = quote_and_escape(label);
        fprintf(ctx->fp, ", \"label\": %s", quoted_label);
        free(quoted_label);
    }

    add_val_labels(ctx, variable, val_labels);
    add_missing_values(ctx, variable);
    
    fprintf(ctx->fp, "}");
    return 0;
}

int handle_variable_dta(int index, readstat_variable_t *variable, const char *val_labels, struct context *ctx) {
    char *type;
    const char *format = readstat_variable_get_format(variable);
    const char *label = readstat_variable_get_label(variable);
    int dta_date = format && (strcmp(format, "%td") == 0) && variable->type == READSTAT_TYPE_INT32;
    int decimals = -1;

    if (variable->type == READSTAT_TYPE_STRING) {
        type = "STRING";
    } else if (dta_date) {
        type = "DATE";
    } else if (variable->type == READSTAT_TYPE_DOUBLE) {
        type = "NUMERIC";
        decimals = extract_decimals(format, '%');
    } else {
        fprintf(stderr, "%s:%d unhandled type %s\n", __FILE__, __LINE__, readstat_type_str(variable->type));
        exit(EXIT_FAILURE);
    }

    if (ctx->count == 0) {
        ctx->count = 1;
        fprintf(ctx->fp, "{\"type\": \"STATA\",\n  \"variables\": [\n");
    } else {
        fprintf(ctx->fp, ",\n");
    }

    fprintf(ctx->fp, "{\"type\": \"%s\", \"name\": \"%s\"", type, variable->name);
    if (decimals > 0) {
        fprintf(ctx->fp, ", \"decimals\": %d", decimals);
    }
    if (label) {
        char* quoted_label = quote_and_escape(label);
        fprintf(ctx->fp, ", \"label\": %s", quoted_label);
        free(quoted_label);
    }

    add_val_labels(ctx, variable, val_labels);
    add_missing_values(ctx, variable);
    fprintf(ctx->fp, "}");
    return 0;
}

int handle_variable(int index, readstat_variable_t *variable, const char *val_labels, void *my_ctx) {
    struct context *ctx = (struct context *)my_ctx;
    if (ctx->input_format == RS_FORMAT_SAV) {
        return handle_variable_sav(index, variable, val_labels, ctx);
    } else if (ctx->input_format == RS_FORMAT_DTA) {
        return handle_variable_dta(index, variable, val_labels, ctx);
    } else {
        fprintf(stderr, "%s:%d unsupported output format %d\n", __FILE__, __LINE__, ctx->input_format);
        exit(EXIT_FAILURE);
    }
}

int pass(struct context *ctx, char *input, char *output, int pass) {
    if (pass==2) {
        FILE* fp = fopen(output, "w");
        if (fp == NULL) {
            fprintf(stderr, "Could not open %s for writing: %s\n", output, strerror(errno));
            exit(EXIT_FAILURE);
        }
        ctx->fp = fp;
    } else {
        ctx->fp = NULL;
    }

    int ret = 0;

    readstat_error_t error = READSTAT_OK;
    readstat_parser_t *parser = readstat_parser_init();
    if (pass == 1) {
        readstat_set_value_label_handler(parser, &handle_value_label);
    } else if (pass == 2) {
        readstat_set_variable_handler(parser, &handle_variable);
    }
    
    const char *filename = input;
    size_t len = strlen(filename);

    if (len < sizeof(".dta") -1) {
        fprintf(stderr, "Unknown input format\n");
        ret = 1;
        goto cleanup;
    }

    if (strncmp(filename + len - 4, ".sav", 4) == 0) {
        fprintf(stdout, "parsing sav file\n");
        error = readstat_parse_sav(parser, input, ctx);
    } else if (strncmp(filename + len - 4, ".dta", 4) == 0) {
        fprintf(stdout, "parsing dta file\n");
        error = readstat_parse_dta(parser, input, ctx);
    } else {
        fprintf(stderr, "Unsupported input format\n");
        ret = 1;
        goto cleanup;
    }

    if (error != READSTAT_OK) {
        fprintf(stderr, "Error processing %s: %s (%d)\n", input, readstat_error_message(error), error);
        ret = 1;
    } else {
        if (ctx->fp) {
            fprintf(ctx->fp, "]}\n");
            fprintf(ctx->fp, "\n");
        }
    }

cleanup: readstat_parser_free(parser);

    if (ctx->fp) {
        fclose(ctx->fp);
    }
    if (pass==2 && ctx->variable_count >=1) {
        for (int i=0; i<ctx->variable_count; i++) {
            readstat_label_set_t * label_set = &ctx->label_set[i];
            for (int j=0; j<label_set->value_labels_count; j++) {
                readstat_value_label_t* value_label = &label_set->value_labels[j];
                if (value_label->string_key) {
                    free(value_label->string_key);
                }
                if (value_label->label) {
                    free(value_label->label);
                }
            }
            free(label_set->value_labels);
        }
        free(ctx->label_set);
    }

    fprintf(stdout, "pass %d done\n", pass);
    return ret;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input-filename.(dta|sav)> <output-metadata.json>\n", argv[0]);
        return 1;
    }
    int ret = 0;
    struct context ctx;
    memset(&ctx, 0, sizeof(struct context));
    ctx.input_format = format(argv[1]);

    ret = pass(&ctx, argv[1], argv[2], 1);
    if (!ret) {
        ret = pass(&ctx, argv[1], argv[2], 2);
    }
    printf("extract_metadata exiting\n");
    return ret;
}
