
#include <stdlib.h>

#include "readstat.h"

readstat_io_t *readstat_io_init(const char *filename) {
    readstat_io_t *io = calloc(1, sizeof(readstat_io_t));
    return io;
}

void readstat_io_free(readstat_io_t *io) {
    if (io)
        free(io);
}

readstat_error_t readstat_set_open_handler(readstat_io_t *io, readstat_open_handler open_handler) {
    io->open_handler = open_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_close_handler(readstat_io_t *io, readstat_close_handler close_handler) {
    io->close_handler = close_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_seek_handler(readstat_io_t *io, readstat_seek_handler seek_handler) {
    io->seek_handler = seek_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_read_handler(readstat_io_t *io, readstat_read_handler read_handler) {
    io->read_handler = read_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_update_handler(readstat_io_t *io, readstat_update_handler update_handler) {
    io->update_handler = update_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_io_ctx(readstat_io_t *io, void *io_ctx) {
    io->io_ctx = io_ctx;
    return READSTAT_OK;
}
