
int values_equal(readstat_value_t expected, readstat_value_t received);

void push_error_if_values_differ(rt_parse_ctx_t *ctx, 
        readstat_value_t expected,
        readstat_value_t received,
        const char *msg);

void push_error_if_doubles_differ(rt_parse_ctx_t *ctx,
        double expected,
        double received,
        const char *msg);

void push_error_if_strings_differ(rt_parse_ctx_t *ctx, 
        const char *expected,
        const char *received,
        const char *msg);

void push_error_if_strings_differ_n(rt_parse_ctx_t *ctx, 
        const char *expected,
        const char *received,
        size_t len,
        const char *msg);

void push_error_if_codes_differ(rt_parse_ctx_t *ctx,
        readstat_error_t expected,
        readstat_error_t received);

void print_error(rt_error_t *error);
