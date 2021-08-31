
#include <errno.h>
#include "readstat.h"
#include "readstat_iconv.h"
#include "readstat_convert.h"

int readstat_bad_byte_copy(char **dst_end, size_t *dst_left, const char **src_end, size_t *src_left) {
    /* copy over the bad bytes and continue */
    if (*dst_left < 1) {
        return READSTAT_HANDLER_ABORT;
    }

    memcpy(*dst_end, *src_end, 1);
    *dst_end += 1;
    *src_end += 1;
    *dst_left -= 1;
    *src_left -= 1;

    return READSTAT_HANDLER_OK;
}

int readstat_bad_byte_replace(char **dst_end, size_t *dst_left, const char **src_end, size_t *src_left) {
    /* use the unicode replacement character */
    if (*dst_left < 3) {
        return READSTAT_HANDLER_ABORT;
    }

    (*dst_end)[0] = (char) 0xEF;
    (*dst_end)[1] = (char) 0xBF;
    (*dst_end)[2] = (char) 0xBD;
    *dst_end += 3;
    *src_end += 1;
    *dst_left -= 3;
    *src_left -= 1;

    return READSTAT_HANDLER_OK;
}

int readstat_bad_byte_skip(char **dst_end, size_t *dst_left, const char **src_end, size_t *src_left) {
    /* skip to the next byte */
    *src_end += 1;
    *src_left -= 1;

    return READSTAT_HANDLER_OK;
}

int readstat_bad_byte_cp1252(char **dst_end, size_t *dst_left, const char **src_end, size_t *src_left) {
    /* try converting the rest of the string as WINDOWS-1252, common encoding error */
    iconv_t converter = iconv_open("UTF-8", "WINDOWS-1252");
    if (converter == (iconv_t)-1) {
        return READSTAT_HANDLER_ABORT;
    }

    size_t tmp_left = 1;
    size_t status = iconv(converter, (readstat_iconv_inbuf_t)src_end, &tmp_left, dst_end, dst_left);

    if (status == (size_t)-1) {
        return READSTAT_HANDLER_ABORT;
    }

    *src_left -= 1;

    iconv_close(converter);
    return READSTAT_HANDLER_OK;
}

readstat_error_t readstat_convert(char *dst, size_t dst_len, const char *src, size_t src_len, iconv_t converter, readstat_bad_byte_handler bad_byte_handler) {
    /* strip off spaces from the input because the programs use ASCII space
     * padding even with non-ASCII encoding. */
    while (src_len && src[src_len-1] == ' ') {
        src_len--;
    }
    if (dst_len == 0) {
        return READSTAT_ERROR_CONVERT_LONG_STRING;
    } else if (converter) {
        size_t dst_left = dst_len - 1;
        char *dst_end = dst;
        size_t src_left = src_len;
        const char *src_end = src;
        while (src_left > 0) {
            size_t status = iconv(converter, (readstat_iconv_inbuf_t)&src_end, &src_left, &dst_end, &dst_left);
            if (status == (size_t)-1) {
                if (errno == E2BIG) {
                    return READSTAT_ERROR_CONVERT_LONG_STRING;
                } else if (errno == EILSEQ) { /* EILSEQ indicates an invalid multibyte sequence */
                    if (bad_byte_handler) {
                        int cb_retval = bad_byte_handler(&dst_end, &dst_left, &src_end, &src_left);
                        if (cb_retval == READSTAT_HANDLER_ABORT) {
                            return READSTAT_ERROR_USER_ABORT;
                        }
                    } else {
                        return READSTAT_ERROR_CONVERT_BAD_STRING;
                    }
                } else if (errno != EINVAL) { /* EINVAL indicates improper truncation; accept it */
                    return READSTAT_ERROR_CONVERT;
                } else {
                    /* finish here if an unknown error code is returned */
                    break;
                }
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
