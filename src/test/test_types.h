#include <time.h>

#define RT_MAX_ROWS                 10
#define RT_MAX_COLS                 10
#define RT_MAX_LABEL_SETS            2
#define RT_MAX_NOTES                 2
#define RT_MAX_STRING_REFS           3
#define RT_MAX_NOTE_SIZE           120
#define RT_MAX_VALUE_LABELS          2
#define RT_MAX_STRING               64
#define RT_MAX_VALUE_LABEL_STRING  121

typedef struct rt_label_set_s {
    char                    name[RT_MAX_STRING];
    readstat_type_t         type;

    struct {
        readstat_value_t    value;
        char                label[RT_MAX_VALUE_LABEL_STRING];
    } value_labels[RT_MAX_VALUE_LABELS];
    long                    value_labels_count;
} rt_label_set_t;

typedef struct rt_column_s {
    char                    name[RT_MAX_STRING];
    char                    label[RT_MAX_STRING];
    char                    format[RT_MAX_STRING];
    readstat_alignment_t    alignment;
    readstat_measure_t      measure;
    readstat_type_t         type;
    readstat_value_t        values[RT_MAX_ROWS];

    struct {
        readstat_value_t    lo;
        readstat_value_t    hi;
    } missing_ranges[3];
    long                    missing_ranges_count;

    char                    label_set[RT_MAX_STRING];
} rt_column_t;

typedef struct rt_test_file_s {
    readstat_error_t    write_error;
    long                test_formats;

    char                label[80];
    struct tm           timestamp;
    long                rows;

    rt_column_t         columns[RT_MAX_COLS];
    long                columns_count;

    rt_label_set_t      label_sets[RT_MAX_LABEL_SETS];
    long                label_sets_count;

    char                notes[RT_MAX_NOTES][RT_MAX_NOTE_SIZE];
    long                notes_count;

    char                string_refs[RT_MAX_STRING_REFS][RT_MAX_STRING];
    long                string_refs_count;

    char                fweight[RT_MAX_STRING];
} rt_test_file_t;

typedef struct rt_error_s {
    readstat_value_t    received;
    readstat_value_t    expected;

    rt_test_file_t     *file;
    long                file_format;
    const char         *file_extension;

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

    char            *strings;
    size_t           strings_len;

    long             var_index;
    long             obs_index;

    long             variables_count;
    long             value_labels_count;
    long             notes_count;

    rt_test_file_t  *file;
    long             file_format;
    long             file_format_version;
    const char      *file_extension;

    size_t           max_file_label_len;

    rt_buffer_ctx_t *buffer_ctx;
} rt_parse_ctx_t;
