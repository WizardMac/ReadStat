
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "../readstat.h"
#include "../readstat_dta.h"

#include "test_types.h"
#include "test_error.h"
#include "test_readstat.h"
#include "test_buffer.h"
#include "test_read.h"
#include "test_write.h"

#define MAX_TESTS_PER_GROUP 20

typedef struct rt_test_group_s {
    char             label[80];
    rt_test_file_t   tests[MAX_TESTS_PER_GROUP];
} rt_test_group_t;

rt_test_group_t _test_groups[] = {
    {
        .label = "Long strings in DTA 117/118",
        .tests = {
            {
                .label = "300-byte string in newer DTA file",
                .test_formats = RT_FORMAT_DTA_117 | RT_FORMAT_DTA_118,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_STRING,
                        .values = { 
                            { .type = READSTAT_TYPE_STRING, .v = 
                                { .string_value = /* 300 bytes long */
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                }
                            }
                        }
                    }
                }
            },
        },
    },
    {
        .label = "Illegal column names",
        .tests = {
            {
                .label = "DTA column name begins with number",
                .write_error = READSTAT_ERROR_NAME_BEGINS_WITH_ILLEGAL_CHARACTER,
                .test_formats = RT_FORMAT_DTA,
                .rows = 0,
                .columns = {
                    {
                        .name = "1var",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "DTA column name contains dollar sign",
                .write_error = READSTAT_ERROR_NAME_CONTAINS_ILLEGAL_CHARACTER,
                .test_formats = RT_FORMAT_DTA,
                .rows = 0,
                .columns = {
                    {
                        .name = "var$",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "DTA column name is a reserved word",
                .write_error = READSTAT_ERROR_NAME_IS_RESERVED_WORD,
                .test_formats = RT_FORMAT_DTA,
                .rows = 0,
                .columns = {
                    {
                        .name = "double",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "DTA column name is a reserved pattern",
                .write_error = READSTAT_ERROR_NAME_IS_RESERVED_WORD,
                .test_formats = RT_FORMAT_DTA,
                .rows = 0,
                .columns = {
                    {
                        .name = "str123",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            }
        }
    },
    {
        .label = "Tagged missing values",
        .tests = {
            {
                .label = "SAV tagged missing values",
                .write_error = READSTAT_ERROR_TAGGED_VALUES_NOT_SUPPORTED,
                .test_formats = RT_FORMAT_SAV,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'a' } 
                        }
                    }
                }
            },

            {
                .label = "DTA out-of-range tagged missing values",
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .test_formats = RT_FORMAT_DTA,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .tag = '$' } 
                        }
                    }
                }
            },

            {
                .label = "DTA in-range tagged missing doubles",
                .test_formats = RT_FORMAT_DTA,
                .rows = 6,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'a' },
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'b' },
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'c' },
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'x' },
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'y' },
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'z' }
                        }
                    }
                }
            },

            {
                .label = "DTA in-range tagged missing floats",
                .test_formats = RT_FORMAT_DTA,
                .rows = 6,
                .columns = {
                    {
                        .name = "var2",
                        .type = READSTAT_TYPE_FLOAT,
                        .values = { 
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'a' },
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'b' },
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'c' },
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'x' },
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'y' },
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'z' }
                        }
                    }
                }
            },

            { 
                .test_formats = RT_FORMAT_DTA,
                .label = "DTA in-range tagged missing int32s",
                .rows = 6,
                .columns = {
                    {
                        .name = "var3",
                        .type = READSTAT_TYPE_INT32,
                        .values = { 
                            { .type = READSTAT_TYPE_INT32, .tag = 'a' },
                            { .type = READSTAT_TYPE_INT32, .tag = 'b' },
                            { .type = READSTAT_TYPE_INT32, .tag = 'c' },
                            { .type = READSTAT_TYPE_INT32, .tag = 'x' },
                            { .type = READSTAT_TYPE_INT32, .tag = 'y' },
                            { .type = READSTAT_TYPE_INT32, .tag = 'z' }
                        }
                    }
                }
            },

            { 
                .label = "DTA in-range tagged missing int16s",
                .test_formats = RT_FORMAT_DTA,
                .rows = 6,
                .columns = {
                    {
                        .name = "var4",
                        .type = READSTAT_TYPE_INT16,
                        .values = { 
                            { .type = READSTAT_TYPE_INT16, .tag = 'a' },
                            { .type = READSTAT_TYPE_INT16, .tag = 'b' },
                            { .type = READSTAT_TYPE_INT16, .tag = 'c' },
                            { .type = READSTAT_TYPE_INT16, .tag = 'x' },
                            { .type = READSTAT_TYPE_INT16, .tag = 'y' },
                            { .type = READSTAT_TYPE_INT16, .tag = 'z' }
                        }
                    }
                }
            },

            {
                .label = "DTA in-range tagged missing int8s",
                .test_formats = RT_FORMAT_DTA,
                .rows = 6,
                .columns = {
                    {
                        .name = "var5",
                        .type = READSTAT_TYPE_CHAR,
                        .values = { 
                            { .type = READSTAT_TYPE_CHAR, .tag = 'a' },
                            { .type = READSTAT_TYPE_CHAR, .tag = 'b' },
                            { .type = READSTAT_TYPE_CHAR, .tag = 'c' },
                            { .type = READSTAT_TYPE_CHAR, .tag = 'x' },
                            { .type = READSTAT_TYPE_CHAR, .tag = 'y' },
                            { .type = READSTAT_TYPE_CHAR, .tag = 'z' }
                        }
                    }
                }
            }
        },
    },

    {
        .label = "Out-of-range values",
        .tests = {
            {
                .label = "DTA out-of-range double value",
                .test_formats = RT_FORMAT_DTA,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = HUGE_VAL } } 
                        }
                    }
                }
            },

            {
                .label = "DTA out-of-range float value",
                .test_formats = RT_FORMAT_DTA,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_FLOAT,
                        .values = { 
                            { .type = READSTAT_TYPE_FLOAT, .v = { .float_value = HUGE_VALF } } 
                        }
                    }
                }
            },

            {
                .label = "DTA out-of-range int32 value",
                .test_formats = RT_FORMAT_DTA,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,

                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT32,
                        .values = { 
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = DTA_MAX_INT32+1 } } 
                        }
                    }
                }
            },

            {
                .label = "DTA out-of-range int16 value",
                .test_formats = RT_FORMAT_DTA,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT16,
                        .values = { { .type = READSTAT_TYPE_INT16, .v = { .i16_value = DTA_MAX_INT16+1 } } }
                    }
                }
            },
            {
                .label = "DTA out-of-range int8 value",
                .test_formats = RT_FORMAT_DTA,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_CHAR,
                        .values = { { .type = READSTAT_TYPE_CHAR, .v = { .char_value = DTA_MAX_CHAR+1 } } }
                    }
                }
            },
        }
    },

    {
        .label = "Generic tests",
        .tests = {
            {
                .label = "Generic test file with all column types",
                .test_formats = RT_FORMAT_ALL,
                .write_error = READSTAT_OK,
                .rows = 5,
                .columns = {
                    { 
                        .name = "var1",
                        .label = "Double-precision variable",
                        .type = READSTAT_TYPE_DOUBLE,
                        .alignment = READSTAT_ALIGNMENT_CENTER,
                        .measure = READSTAT_MEASURE_SCALE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 100.0 } }, 
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 10.0 } }, 
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -3.14159, } }, 
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = NAN } }, 
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -HUGE_VAL } }
                        }
                    },
                    { 
                        .name = "var2",
                        .label = "Single-precision variable",
                        .type = READSTAT_TYPE_FLOAT,
                        .alignment = READSTAT_ALIGNMENT_CENTER,
                        .measure = READSTAT_MEASURE_SCALE,
                        .values = { 
                            { .type = READSTAT_TYPE_FLOAT, .v = { .float_value = 20.0 } }, 
                            { .type = READSTAT_TYPE_FLOAT, .v = { .float_value = 15.0 } },
                            { .type = READSTAT_TYPE_FLOAT, .v = { .float_value = 3.14159 } },
                            { .type = READSTAT_TYPE_FLOAT, .v = { .float_value = NAN } },
                            { .type = READSTAT_TYPE_FLOAT, .v = { .float_value = -HUGE_VALF } }
                        }
                    },
                    { 
                        .name = "var3",
                        .label = "Int32 variable",
                        .type = READSTAT_TYPE_INT32,
                        .alignment = READSTAT_ALIGNMENT_CENTER,
                        .measure = READSTAT_MEASURE_SCALE,
                        .values = { 
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = 20 } },
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = 15 } },
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = -281817 } }, 
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = DTA_MAX_INT32 } },
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = INT32_MIN } }
                        }
                    },
                    { 
                        .name = "var4",
                        .label = "Int16 variable",
                        .type = READSTAT_TYPE_INT16,
                        .alignment = READSTAT_ALIGNMENT_CENTER,
                        .measure = READSTAT_MEASURE_SCALE,
                        .values = { 
                            { .type = READSTAT_TYPE_INT16, .v = { .i16_value = 20 } }, 
                            { .type = READSTAT_TYPE_INT16, .v = { .i16_value = 15 } }, 
                            { .type = READSTAT_TYPE_INT16, .v = { .i16_value = -28117 } }, 
                            { .type = READSTAT_TYPE_INT16, .v = { .i16_value = DTA_MAX_INT16 } },
                            { .type = READSTAT_TYPE_INT16, .v = { .i16_value = INT16_MIN } }
                        }
                    },
                    { 
                        .name = "var5",
                        .label = "Int8 variable",
                        .type = READSTAT_TYPE_CHAR,
                        .alignment = READSTAT_ALIGNMENT_CENTER,
                        .measure = READSTAT_MEASURE_SCALE,
                        .values = { 
                            { .type = READSTAT_TYPE_CHAR, .v = { .char_value = 20 } }, 
                            { .type = READSTAT_TYPE_CHAR, .v = { .char_value = 15 } }, 
                            { .type = READSTAT_TYPE_CHAR, .v = { .char_value = -28 } }, 
                            { .type = READSTAT_TYPE_CHAR, .v = { .char_value = DTA_MAX_CHAR } },
                            { .type = READSTAT_TYPE_CHAR, .v = { .char_value = INT8_MIN } }
                        }
                    },
                    { 
                        .name = "var6",
                        .label = "String variable",
                        .type = READSTAT_TYPE_STRING,
                        .alignment = READSTAT_ALIGNMENT_LEFT,
                        .measure = READSTAT_MEASURE_ORDINAL,
                        .values = { 
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "Hello" } }, 
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "Goodbye" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "Goodbye" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "Goodbye" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "" } },
                        }
                    }
                }
            }
        }
    }
};

int main(int argc, char *argv[]) {
    rt_buffer_t *buffer = buffer_init();
    readstat_error_t error = READSTAT_OK;

    int g, t, f;

    for (g=0; g<sizeof(_test_groups)/sizeof(_test_groups[0]); g++) {
        for (t=0; t<MAX_TESTS_PER_GROUP && _test_groups[g].tests[t].label[0]; t++) {
            rt_test_file_t *file = &_test_groups[g].tests[t];
            int c;
            for (c=0; c<RT_MAX_COLS; c++) {
                if (!file->columns[c].name[0])
                    break;
                file->columns_count++;
            }
            rt_parse_ctx_t *parse_ctx = parse_ctx_init(buffer, file);

            for (f=RT_FORMAT_DTA_104; f<=RT_FORMAT_SAV; f*=2) {
                if (!(file->test_formats & f))
                    continue;

                parse_ctx_reset(parse_ctx, f);

                error = write_file_to_buffer(file, buffer, f);
                if (error != file->write_error) {
                    push_error_if_codes_differ(parse_ctx, file->write_error, error);
                    error = READSTAT_OK;
                    continue;
                }
                if (error != READSTAT_OK) {
                    error = READSTAT_OK;
                    continue;
                }

                error = read_file(parse_ctx, f);
                if (error != READSTAT_OK)
                    goto cleanup;
            }

            if (parse_ctx->errors_count) {
                int i;
                for (i=0; i<parse_ctx->errors_count; i++) {
                    print_error(&parse_ctx->errors[i]);
                }
                return 1;
            }

            free(parse_ctx);
        }
    }

cleanup:
    if (error != READSTAT_OK) {
        printf("Error running test \"%s\" (format=0x%04x): %s\n", 
                _test_groups[g].tests[t].label,
                f, readstat_error_message(error));
        return 1;
    }

    return 0;
}
