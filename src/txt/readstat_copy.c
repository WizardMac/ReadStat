#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void readstat_copy(char *buf, size_t buf_len, unsigned char *str_start, size_t str_len) {
    size_t this_len = str_len;
    if (this_len >= buf_len) {
        this_len = buf_len - 1;
    }
    memcpy(buf, str_start, this_len);
    buf[this_len] = '\0';
}

void readstat_copy_lower(char *buf, size_t buf_len, unsigned char *str_start, size_t str_len) {
    int i;
    readstat_copy(buf, buf_len, str_start, str_len);
    for (i=0; i<buf_len && buf[i]; i++)
        buf[i] = tolower(buf[i]);
}
