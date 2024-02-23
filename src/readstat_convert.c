
#include <errno.h>
#include "readstat.h"
#include "readstat_iconv.h"
#include "readstat_convert.h"

readstat_error_t readstat_convert(char *dst, size_t dst_len, const char *src, size_t src_len, iconv_t converter) {
    /* strip off spaces from the input because the programs use ASCII space
     * padding even with non-ASCII encoding. */
    while (src_len && (src[src_len-1] == ' ' || src[src_len-1] == '\0')) {
        src_len--;
    }
    if (dst_len == 0) {
        return READSTAT_ERROR_CONVERT_LONG_STRING;
    } else if (converter) {
        size_t dst_left = dst_len - 1;
        char *dst_end = dst;
        size_t status = iconv(converter, (readstat_iconv_inbuf_t)&src, &src_len, &dst_end, &dst_left);
        if (status == (size_t)-1) {
            if (errno == E2BIG) { /* E2BIG indicates that the output buffer is not large enough */
                return READSTAT_ERROR_CONVERT_LONG_STRING;
            } else if (errno == EILSEQ) { /* EILSEQ indicates an invalid multibyte sequence */
                return READSTAT_ERROR_CONVERT_BAD_STRING;
            } else if (errno != EINVAL) { /* EINVAL indicates improper truncation; accept it */
                return READSTAT_ERROR_CONVERT;
            }
        }
        dst[dst_len - dst_left - 1] = '\0';
    } else if (src_len + 1 > dst_len) {
        return READSTAT_ERROR_CONVERT_LONG_STRING;
    } else {
        memcpy(dst, src, src_len);
        dst[src_len] = '\0';
    }
    return READSTAT_OK;
}

int readstat_invalid_string_info(char *dst, size_t dst_len, const char *src, size_t src_len, int obs_index, readstat_variable_t *variable, void *ctx) {
    /* show information about the invalid string and exit */
    printf("Invalid string in variable %s, row %d: \"%s\"\n", variable->name, obs_index, src);

    return READSTAT_HANDLER_ABORT;
}

int readstat_invalid_string_copy(char *dst, size_t dst_len, const char *src, size_t src_len, int obs_index, readstat_variable_t *variable, void *ctx) {
    /* copy over the string unedited and continue */

    /* strip off spaces from the input because the programs use ASCII space
     * padding even with non-ASCII encoding. */
    while (src_len && src[src_len-1] == ' ') {
        src_len--;
    }

    if (src_len + 1 > dst_len) {
        return READSTAT_HANDLER_ABORT;
    }

    memcpy(dst, src, src_len);
    dst[src_len] = '\0';

    return READSTAT_HANDLER_OK;
}

int readstat_invalid_string_skip(char *dst, size_t dst_len, const char *src, size_t src_len, int obs_index, readstat_variable_t *variable, void *ctx) {
    /* skip the invalid string */
    dst[0] = '\0';

    return READSTAT_HANDLER_OK;
}

int readstat_invalid_string_utf8(char *dst, size_t dst_len, const char *src, size_t src_len, int obs_index, readstat_variable_t *variable, void *ctx) {
    /* treat string as utf-8 and use the unicode replacement character for any invalid bytes */

    /* strip off spaces from the input because the programs use ASCII space
     * padding even with non-ASCII encoding. */
    while (src_len && src[src_len-1] == ' ') {
        src_len--;
    }

    iconv_t converter = iconv_open("UTF-8", "UTF-8");
    if (converter == (iconv_t)-1) {
        return READSTAT_HANDLER_ABORT;
    }

    size_t dst_left = dst_len - 1;
    char *dst_end = dst;
    size_t src_left = src_len;
    const char *src_end = src;
    while (src_left > 0) {
        size_t status = iconv(converter, (readstat_iconv_inbuf_t)&src_end, &src_left, &dst_end, &dst_left);
        if (status == (size_t)-1) {
            if (errno == E2BIG) { /* E2BIG indicates that the output buffer is not large enough */
                return READSTAT_HANDLER_ABORT;
            } else if (errno == EILSEQ) { /* EILSEQ indicates an invalid multibyte sequence */
                if (dst_left < 3) {
                    return READSTAT_HANDLER_ABORT;
                }

                dst_end[0] = (char) 0xEF;
                dst_end[1] = (char) 0xBF;
                dst_end[2] = (char) 0xBD;
                dst_end += 3;
                src_end += 1;
                dst_left -= 3;
                src_left -= 1;
            } else if (errno != EINVAL) { /* EINVAL indicates improper truncation; accept it */
                return READSTAT_HANDLER_ABORT;
            } else {
                /* finish here and accept conversion if EINVAL is returned */
                break;
            }
        }
    }
    dst[dst_len - dst_left - 1] = '\0';

    iconv_close(converter);
    return READSTAT_HANDLER_OK;
}

int readstat_invalid_string_cp1252(char *dst, size_t dst_len, const char *src, size_t src_len, int obs_index, readstat_variable_t *variable, void *ctx) {
    /* try converting the rest of the string as WINDOWS-1252, common encoding error */
    while (src_len && src[src_len-1] == ' ') {
        src_len--;
    }

    iconv_t converter = iconv_open("UTF-8", "WINDOWS-1252");
    if (converter == (iconv_t)-1) {
        return READSTAT_HANDLER_ABORT;
    }

    size_t dst_left = dst_len - 1;
    char *dst_end = dst;
    size_t status = iconv(converter, (readstat_iconv_inbuf_t)&src, &src_len, &dst_end, &dst_left);

    if (status == (size_t)-1) {
        return READSTAT_HANDLER_ABORT;
    }
    dst[dst_len - dst_left - 1] = '\0';

    iconv_close(converter);
    return READSTAT_HANDLER_OK;
}

