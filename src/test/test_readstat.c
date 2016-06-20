
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

rt_file_t _huge_double_fail = {
    .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,

    .test_formats = RT_FORMAT_DTA,

    .columns = {
        {
            .name = "var1",
            .type = READSTAT_TYPE_DOUBLE,
            .double_values = { HUGE_VAL }
        }
    }
};

rt_file_t _huge_float_fail = {
    .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,

    .test_formats = RT_FORMAT_DTA,

    .columns = {
        {
            .name = "var1",
            .type = READSTAT_TYPE_FLOAT,
            .float_values = { HUGE_VALF }
        }
    }
};

rt_file_t _huge_int32_fail = {
    .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,

    .test_formats = RT_FORMAT_DTA,

    .columns = {
        {
            .name = "var1",
            .type = READSTAT_TYPE_INT32,
            .i32_values = { DTA_MAX_INT32+1 }
        }
    }
};

rt_file_t _huge_int16_fail = {
    .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,

    .test_formats = RT_FORMAT_DTA,

    .columns = {
        {
            .name = "var1",
            .type = READSTAT_TYPE_INT16,
            .i16_values = { DTA_MAX_INT16+1 }
        }
    }
};

rt_file_t _huge_int8_fail = {
    .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,

    .test_formats = RT_FORMAT_DTA,

    .columns = {
        {
            .name = "var1",
            .type = READSTAT_TYPE_CHAR,
            .i8_values = { DTA_MAX_CHAR+1 }
        }
    }
};

rt_file_t _test_file1  = {
    .write_error = READSTAT_OK,
    
    .test_formats = RT_FORMAT_ALL,

    .columns = {
        { 
            .name = "var1",
            .label = "Double-precision variable",
            .type = READSTAT_TYPE_DOUBLE,
            .alignment = READSTAT_ALIGNMENT_CENTER,
            .measure = READSTAT_MEASURE_SCALE,
            .double_values = { 100.0, 10.0, -3.14159, NAN, -HUGE_VAL }
        },
        { 
            .name = "var2",
            .label = "Single-precision variable",
            .type = READSTAT_TYPE_FLOAT,
            .alignment = READSTAT_ALIGNMENT_CENTER,
            .measure = READSTAT_MEASURE_SCALE,
            .float_values = { 20.0, 15.0, 3.14159, NAN }
        },
        { 
            .name = "var3",
            .label = "Int32 variable",
            .type = READSTAT_TYPE_INT32,
            .alignment = READSTAT_ALIGNMENT_CENTER,
            .measure = READSTAT_MEASURE_SCALE,
            .i32_values = { 20, 15, -281817, DTA_MAX_INT32 }
        },
        { 
            .name = "var4",
            .label = "Int16 variable",
            .type = READSTAT_TYPE_INT16,
            .alignment = READSTAT_ALIGNMENT_CENTER,
            .measure = READSTAT_MEASURE_SCALE,
            .i16_values = { 20, 15, -28117, DTA_MAX_INT16 }
        },
        { 
            .name = "var5",
            .label = "Int8 variable",
            .type = READSTAT_TYPE_CHAR,
            .alignment = READSTAT_ALIGNMENT_CENTER,
            .measure = READSTAT_MEASURE_SCALE,
            .i8_values = { 20, 15, -28, DTA_MAX_CHAR }
        },
        { 
            .name = "var6",
            .label = "String variable",
            .type = READSTAT_TYPE_STRING,
            .alignment = READSTAT_ALIGNMENT_LEFT,
            .measure = READSTAT_MEASURE_ORDINAL,
            .string_values = { "Hello", "Goodbye" }
        }
    }
};

int main(int argc, char *argv[]) {
    rt_buffer_t *buffer = buffer_init();
    readstat_error_t error = READSTAT_OK;

    rt_file_t *files[] = { 
        &_huge_double_fail, 
        &_huge_float_fail, 
        &_huge_int32_fail,
        &_huge_int16_fail,
        &_huge_int8_fail,
        &_test_file1 };

    int j;
    int f;

    for (j=0; j<sizeof(files)/sizeof(files[0]); j++) {
        rt_file_t *file = files[j];
        int c;
        for (c=0; c<RT_MAX_COLS; c++) {
            if (!file->columns[c].name[0])
                break;
            file->columns_count++;
        }
        rt_parse_ctx_t *parse_ctx = parse_ctx_init(buffer, j, file);

        for (f=RT_FORMAT_DTA; f<=RT_FORMAT_SAV; f*=2) {
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

cleanup:
    if (error != READSTAT_OK) {
        printf("Error running test (j=%d f=%d): %s\n", j, f, readstat_error_message(error));
        return 1;
    }

    return 0;
}
