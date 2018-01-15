
int rt_open_handler(const char *path, void *io_ctx);
int rt_close_handler(void *io_ctx);
readstat_off_t rt_seek_handler(readstat_off_t offset,
        readstat_io_flags_t whence, void *io_ctx);
ssize_t rt_read_handler(void *buf, size_t nbytes, void *io_ctx);
readstat_error_t rt_update_handler(long file_size, readstat_progress_handler progress_handler,
        void *user_ctx, void *io_ctx);
