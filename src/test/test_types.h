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
#define MAX_TESTS_PER_GROUP 20


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
    int                     display_width;
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
    char                table_name[32];
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

typedef struct rt_test_group_s {
    char             label[80];
    rt_test_file_t   tests[MAX_TESTS_PER_GROUP];
} rt_test_group_t;

typedef struct rt_test_args_s {
    long             row_limit;
    long             row_offset;    
} rt_test_args_t;


typedef struct rt_error_s {
    readstat_value_t received;
    readstat_value_t expected;

    rt_test_file_t  *file;
    long             file_format;
    const char      *file_extension;

    size_t           pos;
    long             var_index;
    long             obs_index;
    char             msg[256];
} rt_error_t;

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

    rt_test_args_t  *args;

    rt_test_file_t  *file;
    long             file_format;
    long             file_format_version;
    const char      *file_extension;

    size_t           max_file_label_len;
    size_t           max_table_name_len;

    void            *buffer_ctx;
} rt_parse_ctx_t;
