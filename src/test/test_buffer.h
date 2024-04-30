
typedef struct rt_buffer_s {
    size_t      used;
    size_t      size;
    char       *bytes;
} rt_buffer_t;

typedef struct rt_buffer_ctx_s {
    rt_buffer_t  *buffer;
    size_t       pos;
} rt_buffer_ctx_t;

rt_buffer_t *buffer_init(void);
void buffer_reset(rt_buffer_t *buffer);
void buffer_grow(rt_buffer_t *buffer, size_t len);
void buffer_free(rt_buffer_t *buffer);

rt_buffer_ctx_t *buffer_ctx_init(rt_buffer_t *buffer);
void buffer_ctx_reset(rt_buffer_ctx_t *buffer_ctx);
