[![Travis CI build status](https://travis-ci.org/WizardMac/ReadStat.svg?branch=master)](https://travis-ci.org/WizardMac/ReadStat)
[![Appveyor build status](https://ci.appveyor.com/api/projects/status/76ctatpy3grlrd9x/branch/master?svg=true)](https://ci.appveyor.com/project/evanmiller/readstat/branch/master)

ReadStat: Read (and write) data sets from R, SAS, Stata, and SPSS
--

Originally developed for [Wizard](http://www.wizardmac.com/), ReadStat is a
command-line tool and MIT-licensed C library for reading files from popular
stats packages. Supported formats include:

* R: RData and RDS
* SAS: SAS7BDAT and SAS7BCAT
* Stata: DTA
* SPSS: POR and SAV

There is also write support for the DTA, POR, and SAV formats. At the moment,
the ReadStat command-line tool works only with the non-R formats.

Installation
==

Bootstrap autotools by running ./autogen.sh and then proceed as usual:

    ./configure
    make
    sudo make install

Windows specific notes
--

You need to install and configure an msys2 environment to compile ReadStat.

First, download and install msys2 from [here](https://msys2.github.io/). Make sure you update your initial msys2 installation as described on that page.

Second, install a number of additional packages at the msys2 command line:

    pacman -S autoconf automake libtool mingw-w64-x86_64-toolchain ingw-w64-x86_64-cmake mingw-w64-x86_64-libiconv

Finally, start a MINGW command line (not the msys2 prompt!) and follow the general install instructions for this package.

Command-line Usage
==

Standard usage:

    readstat <input file> <output file>

Where:

* `<input file>` ends with `.dta`, `.por`, `.sav`, or `.sas7bdat`, and
* `<output file>` ends with `.dta`, `.por`, `.sav`, or `.csv`

If [libxlsxwriter](http://libxlsxwriter.github.io) is found at compile-time, an
XLSX file (ending in `.xlsx`) can be written instead.

Note that ReadStat will not overwrite existing files, so if you get a "File
exists" error, delete the file you intend to replace.

If you have a SAS catalog file containing the data set's value labels, a second
invocation style is supported:

    readstat <input file> <catalog file> <output file>

Where:

* `<input file>` ends with `.sas7bdat`
* `<catalog file>` ends with `.sas7bcat`
* `<output file>` ends with `.dta`, `.por`, `.sav`, or `.csv`

If the file conversion succeeds, ReadStat will report the number of rows and
variables converted, e.g.

    Converted 111 variables and 160851 rows in 12.36 seconds

At the moment value labels are supported, but the finer nuances of converting
format strings (e.g. `%8.2g`) are not.

Library Usage
==

The ReadStat API is callback-based. It uses very little memory, and is suitable
for programs with progress bars.  ReadStat uses
[iconv](https://en.wikipedia.org/wiki/Iconv) to automatically transcode
text data into UTF-8, so you don't have to worry about character encodings. 

See src/readstat.h for the complete API. In general you'll provide a filename
and a set of optional callback functions for handling various information and
data found in the file. It's up to the user to store this information in an
appropriate data structure. If a context pointer is passed to the parse_* functions,
it will be made available to the various callback functions.

Callback functions should return 0 on success. Returning a non-zero value will
abort the parsing process.

Example: Return the number of records in a DTA file.

```c
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
```

Example: Convert a DTA to a tab-separated file.

```c
#include "readstat.h"

int handle_info(int obs_count, int var_count, void *ctx) {
    int *my_var_count = (int *)ctx;
    
    *my_var_count = var_count;

    return 0;
}

int handle_variable(int index, readstat_variable_t *variable, 
    const char *val_labels, void *ctx) {
    int *my_var_count = (int *)ctx;

    printf("%s", readstat_variable_get_name(variable));
    if (index == *my_var_count - 1) {
        printf("\n");
    } else {
        printf("\t");
    }

    return 0;
}

int handle_value(int obs_index, int var_index, readstat_value_t value, void *ctx) {
    int *my_var_count = (int *)ctx;
    readstat_type_t type = readstat_value_type(value);
    if (!readstat_value_is_missing(value)) {
        if (type == READSTAT_TYPE_STRING) {
            printf("%s", readstat_string_value(value));
        } else if (type == READSTAT_TYPE_INT8) {
            printf("%hhd", readstat_int8_value(value));
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
```

Language Bindings
==

* Julia: [DataRead.jl](https://github.com/WizardMac/DataRead.jl)
* R: [haven](https://github.com/hadley/haven)
