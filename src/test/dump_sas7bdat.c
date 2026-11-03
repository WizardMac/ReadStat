/* Prints a sas7bdat file's variables (name, format, label) and values, with
 * special missing values as .A, .B, ..., for comparison in tests. */

#include <stdio.h>

#include "../readstat.h"

static int handle_variable(int index, readstat_variable_t *variable,
        const char *val_labels, void *ctx) {
    const char *format = readstat_variable_get_format(variable);
    const char *label = readstat_variable_get_label(variable);
    printf("variable %s format=%s label=%s\n", readstat_variable_get_name(variable),
            format ? format : "", label ? label : "");
    return READSTAT_HANDLER_OK;
}

static int handle_value(int obs_index, readstat_variable_t *variable,
        readstat_value_t value, void *ctx) {
    printf("%d %s ", obs_index, readstat_variable_get_name(variable));
    if (readstat_value_is_tagged_missing(value)) {
        printf(".%c\n", readstat_value_tag(value));
    } else if (readstat_value_is_system_missing(value)) {
        printf(".\n");
    } else if (readstat_value_type(value) == READSTAT_TYPE_STRING) {
        printf("\"%s\"\n", readstat_string_value(value));
    } else {
        printf("%.15g\n", readstat_double_value(value));
    }
    return READSTAT_HANDLER_OK;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s file.sas7bdat\n", argv[0]);
        return 1;
    }

    readstat_parser_t *parser = readstat_parser_init();
    readstat_set_variable_handler(parser, &handle_variable);
    readstat_set_value_handler(parser, &handle_value);

    readstat_error_t error = readstat_parse_sas7bdat(parser, argv[1], NULL);
    readstat_parser_free(parser);

    if (error != READSTAT_OK) {
        fprintf(stderr, "Error reading %s: %s\n", argv[1], readstat_error_message(error));
        return 1;
    }
    return 0;
}
