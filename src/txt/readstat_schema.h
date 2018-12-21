
typedef struct ds_stata_dct_entry_s {
    int                     row;
    int                     col;
    int                     len;
    readstat_type_t         type;
    char                    varname[33];
    char                    varformat[33];
    char                    varlabel[81];
    char                    decimal_separator;
} readstat_schema_entry_t;

typedef struct readstat_schema_s {
    char                    filename[255];
    int                     rows_per_observation;
    int                     cols_per_observation;
    int                     first_line;
    int                     entry_count;
    char                    field_delimiter;
    readstat_schema_entry_t *entries;
} readstat_schema_t;

void readstat_schema_free(readstat_schema_t *schema);
