
rt_buffer_t *buffer_init();
void buffer_reset(rt_buffer_t *buffer);
void buffer_grow(rt_buffer_t *buffer, size_t len);
void buffer_free(rt_buffer_t *buffer);
