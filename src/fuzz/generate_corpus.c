#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>
#include <sys/stat.h>

#include "../readstat.h"
#include "../readstat_iconv.h"

#include "../stata/readstat_dta.h"

#include "../test/test_buffer.h"
#include "../test/test_types.h"
#include "../test/test_error.h"
#include "../test/test_readstat.h"
#include "../test/test_read.h"
#include "../test/test_write.h"
#include "../test/test_list.h"

#if (defined(_WIN32) || defined(__WIN32__))
#define mkdir(A, B) mkdir(A)
#endif

#define CORPUS_DIR "fuzz/corpus"

static void dump_buffer(rt_buffer_t *buffer, long format, int test_case) {
    char filename[128];
    snprintf(filename, sizeof(filename), CORPUS_DIR "/%s/test-case-%03d", 
            file_extension(format), test_case);

    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror(filename);
    }
    ssize_t bytes_written = fwrite(buffer->bytes, 1, buffer->used, file);
    if (bytes_written < 0) {
        perror(filename);
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    rt_buffer_t *buffer = buffer_init();
    int g, t, f;
    int file_count = 0, test_count = 0;

    if (mkdir(CORPUS_DIR, 0755) == -1 && errno != EEXIST)
        perror(CORPUS_DIR);

    for (f=RT_FORMAT_DTA_104; f<RT_FORMAT_ALL; f*=2) {
        char filename[128];
        snprintf(filename, sizeof(filename), CORPUS_DIR "/%s", file_extension(f));
        if (mkdir(filename, 0755) == -1 && errno != EEXIST)
            perror(filename);
    }

    for (g=0; g<sizeof(_test_groups)/sizeof(_test_groups[0]); g++) {
        for (t=0; t<MAX_TESTS_PER_GROUP && _test_groups[g].tests[t].label[0]; t++) {
            rt_test_file_t *file = &_test_groups[g].tests[t];
            readstat_error_t error = READSTAT_OK;

            for (f=RT_FORMAT_DTA_104; f<RT_FORMAT_ALL; f*=2) {
                if (!(file->test_formats & f))
                    continue;
                if (file->write_error != READSTAT_OK)
                    continue;

                buffer_reset(buffer);

                error = write_file_to_buffer(file, buffer, f);
                if (error != READSTAT_OK) {
                    printf("Error writing to file \"%s\": %s\n", file->label, readstat_error_message(error));
                    exit(1);
                }
                dump_buffer(buffer, f, test_count);
                file_count++;
            }
            test_count++;
        }
    }
    buffer_free(buffer);
    printf("Generated %d corpus files (%d test cases)\n", file_count, test_count);
    return 0;
}
