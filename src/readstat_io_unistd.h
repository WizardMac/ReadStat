
typedef struct unistd_io_ctx_s {
    const char       *filename;
    int               fd;
} unistd_io_ctx_t;

int unistd_open_handler(void *io_ctx);
int unistd_close_handler(void *io_ctx);
readstat_off_t unistd_seek_handler(readstat_off_t offset, readstat_io_flags_t whence, void *io_ctx);
ssize_t unistd_read_handler(void *buf, size_t nbytes, void *io_ctx);
readstat_error_t unistd_update_handler(long file_size, readstat_progress_handler progress_handler, void *user_ctx, void *io_ctx);
readstat_io_t *unistd_io_init(const char *filename);
void unistd_io_free(readstat_io_t *io);
