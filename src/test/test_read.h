
rt_parse_ctx_t *parse_ctx_init(rt_buffer_t *buffer, rt_test_file_t *file);
void parse_ctx_reset(rt_parse_ctx_t *parse_ctx, long file_format);
void parse_ctx_free(rt_parse_ctx_t *parse_ctx);

char *file_extension(long format);
readstat_error_t read_file(rt_parse_ctx_t *parse_ctx, long format);
