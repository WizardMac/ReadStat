#define RT_MAX_ROWS     10
#define RT_MAX_COLS     10
#define RT_MAX_STRING   64

typedef struct rt_error_s {
    readstat_value_t    received;
    readstat_value_t    expected;
    long                file_format;
    long                file_index;
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

typedef struct rt_column_s {
    char                    name[RT_MAX_STRING];
    char                    label[RT_MAX_STRING];
    readstat_alignment_t    alignment;
    readstat_measure_t      measure;
    readstat_types_t        type;
    double                  double_values[RT_MAX_ROWS];
    float                   float_values[RT_MAX_ROWS];
    int8_t                  i8_values[RT_MAX_ROWS];
    int16_t                 i16_values[RT_MAX_ROWS];
    int32_t                 i32_values[RT_MAX_ROWS];
    char                    string_values[RT_MAX_ROWS][RT_MAX_STRING];
} rt_column_t;

typedef struct rt_file_s {
    readstat_error_t    write_error;
    long                test_formats;

    rt_column_t         columns[RT_MAX_COLS];
    long                columns_count;
} rt_file_t;

typedef struct rt_parse_ctx_s {
    rt_error_t      *errors;
    long             errors_count;

    long             var_index;
    long             obs_index;

    long             file_format;
    long             file_index;
    rt_file_t       *file;

    rt_buffer_ctx_t *buffer_ctx;
} rt_parse_ctx_t;
