#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>

#include "../readstat.h"
#include "../readstat_iconv.h"

#include "../stata/readstat_dta.h"

#include "../test/test_types.h"
#include "../test/test_error.h"
#include "../test/test_readstat.h"
#include "../test/test_buffer.h"
#include "../test/test_read.h"
#include "../test/test_write.h"
#include "../test/test_list.h"

static void dump_buffer(rt_buffer_t *buffer, long format) {
    char filename[128];
    snprintf(filename, sizeof(filename), "corpus/%s/dump-%08x", 
            file_extension(format), arc4random());
    mkdir(dirname(filename), 0755);

    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    write(fd, buffer->bytes, buffer->used);
    close(fd);
}

int main(int argc, char *argv[]) {
    rt_buffer_t *buffer = buffer_init();
    int g, t, f;
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
                dump_buffer(buffer, f);
            }
        }
    }
    buffer_free(buffer);
    return 0;
}
