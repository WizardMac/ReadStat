
typedef struct readstat_schema_entry_s {
    int                 row;
    int                 col;
    int                 len;
    int                 skip;
    readstat_variable_t variable;
    char                labelset[32];
    char                decimal_separator;
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

readstat_schema_entry_t *readstat_schema_find_or_create_entry(readstat_schema_t *dct, const char *var_name);
void readstat_schema_free(readstat_schema_t *schema);
