ReadStat: Read data sets from R, SAS, Stata, and SPSS
--

Originally developed for [Wizard](http://www.wizardmac.com/), ReadStat is an
MIT-licensed C library for reading files from popular stats packages. Supported
formats include:

* R: RData and RDS
* SAS: SAS7BDAT and SAS7BCAT
* Stata: DTA
* SPSS: POR and SAV

There is also preliminary write support for the DTA and SAV formats.

The ReadStat API is callback-based. It uses very little memory, and is suitable
for programs with progress bars.  ReadStat uses
[iconv](https://en.wikipedia.org/wiki/Iconv) to automatically transcode
text data into UTF-8, so you don't have to worry about character encodings. 


Installation
==

Tweak the Makefile and then:

    make
    sudo make install


Usage
==

See src/readstat.h for the complete API. In general you'll provide a filename
and a set of optional callback functions for handling various information and
data found in the file. It's up to the user to store this information in an
appropriate data structure. If a context pointer is passed to the parse_* functions,
it will be made available to the various callback functions.

Callback functions should return 0 on success. Returning a non-zero value will
abort the parsing process.

Example: Return the number of records in a DTA file.

    #include "readstat.h"

    int handle_info(int obs_count, int var_count, void *ctx) {
        int *my_count = (int *)ctx;

        *my_count = obs_count;

        return 0;
    }

    int main(int argc, char *argv[]) {
        if (argc != 2) {
            printf("Usage: %s <filename>\n", argv[0]);
            return 1;
        }
        int my_count = 0;
        readstat_error_t error = READSTAT_OK;
        readstat_parser_t *parser = readstat_parser_init();
        readstat_set_info_handler(parser, &handle_info);

        error = readstat_parse_dta(parser, argv[1], &my_count);

        readstat_parser_free(parser);

        if (error != READSTAT_OK) {
            printf("Error processing %s: %d\n", argv[1], error);
            return 1;
        }
        printf("Found %d records\n", my_count);
        return 0;
    }

Example: Convert a DTA to a tab-separated file.

    #include "readstat.h"

    int handle_info(int obs_count, int var_count, void *ctx) {
        int *my_var_count = (int *)ctx;
        
        *my_var_count = var_count;

        return 0;
    }

    int handle_variable(int index, const char *var_name, const char *var_format, const char *var_label, 
        const char *val_labels, readstat_types_t type, void *ctx) {
        int *my_var_count = (int *)ctx;

        printf("%s", var_name);
        if (index == *my_var_count - 1) {
            printf("\n");
        } else {
            printf("\t");
        }

        return 0;
    }

    int handle_value(int obs_index, int var_index, readstat_value_t value, readstat_types_t type, void *ctx) {
        int *my_var_count = (int *)ctx;
        if (!readstat_value_is_missing(value)) {
            if (type == READSTAT_TYPE_STRING) {
                printf("%s", readstat_string_value(value));
            } else if (type == READSTAT_TYPE_CHAR) {
                printf("%hhd", readstat_char_value(value));
            } else if (type == READSTAT_TYPE_INT16) {
                printf("%hd", readstat_int16_value(value));
            } else if (type == READSTAT_TYPE_INT32) {
                printf("%d", readstat_int32_value(value));
            } else if (type == READSTAT_TYPE_FLOAT) {
                printf("%f", readstat_float_value(value));
            } else if (type == READSTAT_TYPE_DOUBLE) {
                printf("%lf", readstat_double_value(value));
            }
        }
        if (var_index == *my_var_count - 1) {
            printf("\n");
        } else {
            printf("\t");
        }

        return 0;
    }

    int main(int argc, char *argv[]) {
        if (argc != 2) {
            printf("Usage: %s <filename>\n", argv[0]);
            return 1;
        }
        int my_var_count = 0;
        readstat_error_t error = READSTAT_OK;
        readstat_parser_t *parser = readstat_parser_init();
        readstat_set_info_handler(parser, &handle_info);
        readstat_set_variable_handler(parser, &handle_variable);
        readstat_set_value_handler(parser, &handle_value);

        error = readstat_parse_dta(parser, argv[1], &my_var_count);

        readstat_parser_free(parser);

        if (error != READSTAT_OK) {
            printf("Error processing %s: %d\n", argv[1], error);
            return 1;
        }
        return 0;
    }


Language Bindings
==

* Julia: [DataRead.jl](https://github.com/WizardMac/DataRead.jl)
* R: [haven](https://github.com/hadley/haven)
