
#include <errno.h>
#include "readstat.h"
#include "readstat_iconv.h"
#include "readstat_convert.h"

readstat_error_t readstat_convert(char *dst, size_t dst_len, const char *src, size_t src_len, iconv_t converter) {
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
                    /* if an invalid multi-byte sequence is found, copy over the
                     * bad bytes and continue */
                    memcpy(dst_end, src_end, 1);
                    dst_end++;
                    src_end++;
                    dst_left++;
                    src_left++;
                } else if (errno != EINVAL) { /* EINVAL indicates improper truncation; accept it */
                    return READSTAT_ERROR_CONVERT;
                } else {
                    /* finish here if an unknown error code is returned */
                    src_left = 0;
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
