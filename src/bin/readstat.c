#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "../readstat.h"
#include "module.h"
#include "modules/mod_readstat.h"
#include "modules/mod_csv.h"

#if HAVE_CSVREADER
#include "modules/produce_csv_column_header.h"
#include "modules/mod_csv_reader.h"
#endif

#if HAVE_XLSXWRITER
#include "modules/mod_xlsx.h"
#endif

#include "format.h"

#define RS_VERSION_STRING  "1.0-prerelease"

#define RS_FORMAT_CAN_WRITE     (RS_FORMAT_DTA | RS_FORMAT_SAV)

typedef struct rs_ctx_s {
    rs_module_t *module;
    void        *module_ctx;
    long         row_count;
    long         var_count;
} rs_ctx_t;

const char *format_name(int format) {
    if (format == RS_FORMAT_DTA)
        return "Stata binary file (DTA)";

    if (format == RS_FORMAT_SAV)
        return "SPSS binary file (SAV)";

    if (format == RS_FORMAT_POR)
        return "SPSS portable file (POR)";

    if (format == RS_FORMAT_SAS_DATA)
        return "SAS data file (SAS7BDAT)";

    if (format == RS_FORMAT_SAS_CATALOG)
        return "SAS catalog file (SAS7BCAT)";
    
    if (format == RS_FORMAT_CSV)
        return "CSV";

    if (format == RS_FORMAT_XPORT)
        return "SAS transport file (XPORT)";

    return "Unknown";
}

int is_catalog(const char *filename) {
    return (format(filename) == RS_FORMAT_SAS_CATALOG);
}

int is_json(const char *filename) {
    return (format(filename) == RS_FORMAT_JSON);
}

int can_read(const char *filename) {
    return (format(filename) != RS_FORMAT_UNKNOWN);
}

rs_module_t *rs_module_for_filename(rs_module_t *modules, long module_count, const char *filename) {
    int i;
    for (i=0; i<module_count; i++) {
        rs_module_t mod = modules[i];
        if (mod.accept(filename))
            return &modules[i];
    }
    return NULL;
}

int can_write(rs_module_t *modules, long modules_count, char *filename) {
    return (rs_module_for_filename(modules, modules_count, filename) != NULL);
}

static void handle_error(const char *msg, void *ctx) {
    fprintf(stderr, "%s", msg);
}

static int handle_fweight(readstat_variable_t *variable, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    if (rs_ctx->module->handle_fweight) {
        return rs_ctx->module->handle_fweight(variable, rs_ctx->module_ctx);
    }
    return READSTAT_HANDLER_OK;
}

static int handle_info(int obs_count, int var_count, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    if (rs_ctx->module->handle_info) {
        return rs_ctx->module->handle_info(obs_count, var_count, rs_ctx->module_ctx);
    }
    return READSTAT_HANDLER_OK;
}

static int handle_metadata(const char *file_label, time_t timestamp, long format_version, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    if (rs_ctx->module->handle_note) {
        return rs_ctx->module->handle_metadata(file_label, timestamp, format_version, rs_ctx->module_ctx);
    }
    return READSTAT_HANDLER_OK;
}

static int handle_note(int note_index, const char *note, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    if (rs_ctx->module->handle_note) {
        return rs_ctx->module->handle_note(note_index, note, rs_ctx->module_ctx);
    }
    return READSTAT_HANDLER_OK;
}

static int handle_value_label(const char *val_labels, readstat_value_t value,
                              const char *label, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    if (rs_ctx->module->handle_value_label) {
        return rs_ctx->module->handle_value_label(val_labels, value, label, rs_ctx->module_ctx);
    }
    return READSTAT_HANDLER_OK;
}

static int handle_variable(int index, readstat_variable_t *variable,
                           const char *val_labels, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    if (rs_ctx->module->handle_variable) {
        return rs_ctx->module->handle_variable(index, variable, val_labels, rs_ctx->module_ctx);
    }
    return READSTAT_HANDLER_OK;
}

static int handle_value(int obs_index, readstat_variable_t *variable, readstat_value_t value, void *ctx) {
    rs_ctx_t *rs_ctx = (rs_ctx_t *)ctx;
    int var_index = readstat_variable_get_index(variable);
    if (var_index == 0) {
        rs_ctx->row_count++;
    }
    if (obs_index == 0) {
        rs_ctx->var_count++;
    }
    if (rs_ctx->module->handle_value) {
        return rs_ctx->module->handle_value(obs_index, variable, value, rs_ctx->module_ctx);
    }
    return READSTAT_HANDLER_OK;
}

readstat_error_t parse_file(readstat_parser_t *parser, const char *input_filename, int input_format, void *ctx) {
    readstat_error_t error = READSTAT_OK;

    if (input_format == RS_FORMAT_DTA) {
        error = readstat_parse_dta(parser, input_filename, ctx);
    } else if (input_format == RS_FORMAT_SAV) {
        error = readstat_parse_sav(parser, input_filename, ctx);
    } else if (input_format == RS_FORMAT_POR) {
        error = readstat_parse_por(parser, input_filename, ctx);
    } else if (input_format == RS_FORMAT_SAS_DATA) {
        error = readstat_parse_sas7bdat(parser, input_filename, ctx);
    } else if (input_format == RS_FORMAT_SAS_CATALOG) {
        error = readstat_parse_sas7bcat(parser, input_filename, ctx);
    } else if (input_format == RS_FORMAT_XPORT) {
        error = readstat_parse_xport(parser, input_filename, ctx);
    }

    return error;
}

static void print_version() {
    fprintf(stderr, "ReadStat version " RS_VERSION_STRING "\n");
}

static void print_usage(const char *cmd) {
    print_version();

    fprintf(stderr, "\n  View a file's metadata:\n");
    fprintf(stderr, "\n     %s input.(dta|por|sav|sas7bdat|xpt)\n", cmd);

    fprintf(stderr, "\n  Convert a file:\n");
    fprintf(stderr, "\n     %s input.(dta|por|sav|sas7bdat|xpt) output.(dta|por|sav|sas7bdat|xpt|csv"
#if HAVE_XLSXWRITER
            "|xlsx"
#endif
            ")\n", cmd);
    fprintf(stderr, "\n  Convert a file if your value labels are stored in a separate SAS catalog file:\n");
    fprintf(stderr, "\n     %s input.sas7bdat catalog.sas7bcat output.(dta|por|sav|csv"
#if HAVE_XLSXWRITER
            "|xlsx"
#endif
            ")\n\n", cmd);
}

static int convert_file(const char *input_filename, const char *catalog_filename, const char *output_filename,
        rs_module_t *modules, int modules_count) {
    readstat_error_t error = READSTAT_OK;
    const char *error_filename = NULL;
    struct timeval start_time, end_time;
    int input_format = format(input_filename);
    rs_module_t *module = rs_module_for_filename(modules, modules_count, output_filename);
    #if HAVE_CSVREADER
    struct csv_metadata csv_meta;
    memset(&csv_meta, 0, sizeof(csv_metadata));
    csv_meta.output_format = format(output_filename);
    #endif

    gettimeofday(&start_time, NULL);

    readstat_parser_t *pass1_parser = readstat_parser_init();
    readstat_parser_t *pass2_parser = readstat_parser_init();

    rs_ctx_t *rs_ctx = calloc(1, sizeof(rs_ctx_t));

    void *module_ctx = module->init(output_filename);

    if (module_ctx == NULL) {
        error = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    rs_ctx->module = module;
    rs_ctx->module_ctx = module_ctx;

    // Pass 1 - Collect fweight and value labels
    readstat_set_error_handler(pass1_parser, &handle_error);
    readstat_set_info_handler(pass1_parser, &handle_info);
    readstat_set_value_label_handler(pass1_parser, &handle_value_label);
    readstat_set_fweight_handler(pass1_parser, &handle_fweight);
    
    if (catalog_filename && input_format != RS_FORMAT_CSV) {
        error = parse_file(pass1_parser, catalog_filename, RS_FORMAT_SAS_CATALOG, rs_ctx);
        error_filename = catalog_filename;
    } else if (catalog_filename && input_format == RS_FORMAT_CSV) {
        #if HAVE_CSVREADER
            error = readstat_parse_csv(pass1_parser, input_filename, catalog_filename, &csv_meta, rs_ctx);
            error_filename = input_filename;
        #else
            fprintf(stderr, "Should never happen... Fix macros\n");
            exit(EXIT_FAILURE);
        #endif
    } else {
        error = parse_file(pass1_parser, input_filename, input_format, rs_ctx);
        error_filename = input_filename;
    }
    if (error != READSTAT_OK)
        goto cleanup;
    
    // Pass 2 - Parse full file
    readstat_set_error_handler(pass2_parser, &handle_error);
    readstat_set_info_handler(pass2_parser, &handle_info);
    readstat_set_metadata_handler(pass2_parser, &handle_metadata);
    readstat_set_note_handler(pass2_parser, &handle_note);
    readstat_set_variable_handler(pass2_parser, &handle_variable);
    readstat_set_value_handler(pass2_parser, &handle_value);

    if (catalog_filename && input_format == RS_FORMAT_CSV) {
        #if HAVE_CSVREADER
            error = readstat_parse_csv(pass2_parser, input_filename, catalog_filename, &csv_meta, rs_ctx);
        #else
            fprintf(stderr, "Should never happen... Fix macros\n");
            exit(EXIT_FAILURE);
        #endif
    } else {
        error = parse_file(pass2_parser, input_filename, input_format, rs_ctx);
    }
    error_filename = input_filename;
    if (error != READSTAT_OK)
        goto cleanup;

    gettimeofday(&end_time, NULL);

    fprintf(stderr, "Converted %ld variables and %ld rows in %.2lf seconds\n",
            rs_ctx->var_count, rs_ctx->row_count, 
            (end_time.tv_sec + 1e-6 * end_time.tv_usec) -
            (start_time.tv_sec + 1e-6 * start_time.tv_usec));

cleanup:
    #if HAVE_CSVREADER
    if (csv_meta.column_width) {
        free(csv_meta.column_width);
        csv_meta.column_width = NULL;
    }
    #endif
    readstat_parser_free(pass1_parser);
    readstat_parser_free(pass2_parser);

    if (module->finish) {
        module->finish(rs_ctx->module_ctx);
    }

    free(rs_ctx);

    if (error != READSTAT_OK) {
        fprintf(stderr, "Error processing %s: %s\n", error_filename, readstat_error_message(error));
        unlink(output_filename);
        return 1;
    }

    return 0;
}

static int dump_info(int obs_count, int var_count, void *ctx) {
    printf("Columns: %d\n", var_count);
    printf("Rows: %d\n", obs_count);
    return 0;
}

static int dump_metadata(const char *file_label, time_t timestamp, long version, void *ctx) {
    if (file_label && file_label[0]) {
        printf("File label: %s\n", file_label);
    }
    if (timestamp) {
        char buffer[128];
        strftime(buffer, sizeof(buffer), "%d %b %Y %H:%M", localtime(&timestamp));
        printf("Timestamp: %s\n", buffer);
    }
    if (version) {
        printf("File format version: %ld\n", version);
    }
    return 0;
}

static int dump_file(const char *input_filename) {
    int input_format = format(input_filename);
    readstat_parser_t *parser = readstat_parser_init();
    readstat_error_t error = READSTAT_OK;

    printf("Format: %s\n", format_name(input_format));

    readstat_set_error_handler(parser, &handle_error);
    readstat_set_info_handler(parser, &dump_info);
    readstat_set_metadata_handler(parser, &dump_metadata);

    error = parse_file(parser, input_filename, input_format, NULL);

    readstat_parser_free(parser);

    if (error != READSTAT_OK) {
        fprintf(stderr, "Error processing %s: %s\n", input_filename, readstat_error_message(error));
        return 1;
    }

    return 0;
}

int main(int argc, char** argv) {
    char *input_filename = NULL;
    char *catalog_filename = NULL;
    char *output_filename = NULL;

    rs_module_t *modules = NULL;
    long modules_count = 2;
    long module_index = 0;

#if HAVE_XLSXWRITER
    modules_count++;
#endif

    modules = calloc(modules_count, sizeof(rs_module_t));

    modules[module_index++] = rs_mod_readstat;
    modules[module_index++] = rs_mod_csv;

#if HAVE_XLSXWRITER
    modules[module_index++] = rs_mod_xlsx;
#endif
    if (argc == 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
        print_version();
        return 0;
    } else if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    } else if (argc == 2) {
        if (!can_read(argv[1])) {
            print_usage(argv[0]);
            return 1;
        }
        input_filename = argv[1];
    } else if (argc == 3) {
        if (!can_read(argv[1]) || !can_write(modules, modules_count, argv[2])) {
            print_usage(argv[0]);
            return 1;
        }
        input_filename = argv[1];
        output_filename = argv[2];
    } else if (argc == 4 && can_read(argv[1]) && is_json(argv[2]) && can_read(argv[2]) && can_write(modules, modules_count, argv[3])) {
        input_filename = argv[1];
        catalog_filename = argv[2];
        output_filename = argv[3];
    } else if (argc == 4) {
        if (!can_read(argv[1]) || !is_catalog(argv[2]) || !can_write(modules, modules_count, argv[3])) {
            print_usage(argv[0]);
            return 1;
        }
        input_filename = argv[1];
        catalog_filename = argv[2];
        output_filename = argv[3];
    } else {
        print_usage(argv[0]);
        return 1;
    }

    int ret;
    if (output_filename) {
        ret = convert_file(input_filename, catalog_filename, output_filename, modules, modules_count);
    } else {
        ret = dump_file(input_filename); 
    }
    free(modules);
    return ret;
}

