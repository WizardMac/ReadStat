
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "../readstat.h"
#include "../readstat_iconv.h"

#include "../stata/readstat_dta.h"

#include "test_buffer.h"
#include "test_types.h"
#include "test_error.h"
#include "test_readstat.h"
#include "test_read.h"
#include "test_write.h"
#include "test_list.h"

static void dump_buffer(rt_buffer_t *buffer, long format) {
    char filename[128];
    snprintf(filename, sizeof(filename), "/tmp/test_readstat.%s", 
            file_extension(format));
#if DEBUG
    printf("Writing file buffer to %s\n", filename);
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    int bytes_written = write(fd, buffer->bytes, buffer->used);
    if (bytes_written != buffer->used)
        printf("Failed to write file!\n");
    close(fd);
#endif
}

int main(int argc, char *argv[]) {
    rt_buffer_t *buffer = buffer_init();
    readstat_error_t error = READSTAT_OK;

    int g, t, f;

    for (g=0; g<sizeof(_test_groups)/sizeof(_test_groups[0]); g++) {
        for (t=0; t<MAX_TESTS_PER_GROUP && _test_groups[g].tests[t].label[0]; t++) {
            rt_test_file_t *file = &_test_groups[g].tests[t];
            rt_parse_ctx_t *parse_ctx = parse_ctx_init(buffer, file);

            for (f=RT_FORMAT_DTA_104; f<RT_FORMAT_ALL; f*=2) {
                if (!(file->test_formats & f))
                    continue;

                int old_errors_count = parse_ctx->errors_count;
                parse_ctx_reset(parse_ctx, f);

                error = write_file_to_buffer(file, buffer, f);
                if (error != file->write_error) {
                    push_error_if_codes_differ(parse_ctx, file->write_error, error);
                    error = READSTAT_OK;
                    continue;
                }
                if (error != READSTAT_OK) {
                    error = READSTAT_OK;
                    continue;
                }

                error = read_file(parse_ctx, f);
                if (error != READSTAT_OK)
                    break;

                if (old_errors_count != parse_ctx->errors_count)
                    dump_buffer(buffer, f);
            }

            if (parse_ctx->errors_count) {
                int i;
                for (i=0; i<parse_ctx->errors_count; i++) {
                    print_error(&parse_ctx->errors[i]);
                }
                parse_ctx_free(parse_ctx);
                buffer_free(buffer);
                return 1;
            }

            parse_ctx_free(parse_ctx);

            if (error != READSTAT_OK)
                goto cleanup;
        }
    }

cleanup:
    if (error != READSTAT_OK) {
        dump_buffer(buffer, f);
        printf("Error running test \"%s\" (format=%s): %s\n", 
                _test_groups[g].tests[t].label,
                file_extension(f), readstat_error_message(error));
        buffer_free(buffer);
        return 1;
    }
    buffer_free(buffer);

    return 0;
}
