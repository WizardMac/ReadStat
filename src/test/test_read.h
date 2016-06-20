
rt_parse_ctx_t *parse_ctx_init(rt_buffer_t *buffer, long file_index, rt_file_t *file);
void parse_ctx_reset(rt_parse_ctx_t *parse_ctx, long file_format);
void parse_ctx_free(rt_parse_ctx_t *parse_ctx);

readstat_error_t read_file(rt_parse_ctx_t *parse_ctx, long format);
