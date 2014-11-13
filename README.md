ReadStat: Read data sets from R, SAS, Stata, and SPSS
--

ReadStat is an MIT-licensed C library for reading files from popular stats
packages. Supported formats include:

* R: RData and RDS
* SAS: SAS7BDAT and SAS7BCAT
* Stata: DTA
* SPSS: POR and SAV

The API is callback-based and is suitable for programs with progress bars.


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
        int error = parse_dta(argv[1], &my_count, &handle_info, NULL, NULL, NULL);
        if (error != 0) {
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
        const char *val_labels, readstat_types_t type, size_t max_len, void *ctx) {
        int *my_var_count = (int *)ctx;

        printf("%s", var_name);
        if (index == *my_var_count - 1) {
            printf("\n");
        } else {
            printf("\t");
        }

        return 0;
    }

    int handle_value(int obs_index, int var_index, void *value, readstat_types_t type, void *ctx) {
        int *my_var_count = (int *)ctx;
        if (value != NULL) {
            if (type == READSTAT_TYPE_STRING) {
                printf("%s", (char *)value);
            } else if (type == READSTAT_TYPE_CHAR) {
                printf("%hhd", *(char *)value);
            } else if (type == READSTAT_TYPE_INT16) {
                printf("%hd", *(short *)value);
            } else if (type == READSTAT_TYPE_INT32) {
                printf("%d", *(int *)value);
            } else if (type == READSTAT_TYPE_FLOAT) {
                printf("%f", *(float *)value);
            } else if (type == READSTAT_TYPE_DOUBLE) {
                printf("%lf", *(double *)value);
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
        int error = parse_dta(argv[1], &my_var_count, &handle_info, &handle_variable, &handle_value, NULL);
        if (error != 0) {
            printf("Error processing %s: %d\n", argv[1], error);
            return 1;
        }
        return 0;
    }

