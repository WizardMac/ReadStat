#define RT_MAX_ROWS     10
#define RT_MAX_COLS     10
#define RT_MAX_STRING   64

typedef struct rt_column_s {
    char                    name[RT_MAX_STRING];
    char                    label[RT_MAX_STRING];
    readstat_alignment_t    alignment;
    readstat_measure_t      measure;
    readstat_types_t        type;
    readstat_value_t        values[RT_MAX_ROWS];
} rt_column_t;

typedef struct rt_test_file_s {
    readstat_error_t    write_error;
    long                test_formats;

    char                label[80];
    long                rows;

    rt_column_t         columns[RT_MAX_COLS];
    long                columns_count;
} rt_test_file_t;

typedef struct rt_error_s {
    readstat_value_t    received;
    readstat_value_t    expected;

    rt_test_file_t     *file;
    long                file_format;

    readstat_off_t      pos;
    long                var_index;
    long                obs_index;
    char                msg[256];
} rt_error_t;

typedef struct rt_buffer_s {
    size_t      used;
    size_t      size;
    char       *bytes;
} rt_buffer_t;

typedef struct rt_buffer_ctx_s {
    rt_buffer_t     *buffer;
    readstat_off_t   pos;
} rt_buffer_ctx_t;

typedef struct rt_parse_ctx_s {
    rt_error_t      *errors;
    long             errors_count;

    long             var_index;
    long             obs_index;

    rt_test_file_t  *file;
    long             file_format;

    rt_buffer_ctx_t *buffer_ctx;
} rt_parse_ctx_t;
